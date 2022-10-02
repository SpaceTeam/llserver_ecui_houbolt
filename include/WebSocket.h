#ifndef WEB_SOCKET_HPP
#define WEB_SOCKET_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <climits>
#include <system_error>

#include <string>
#include <memory>
#include <optional>

#include "utility/RingBuffer.h"
#include "utility/Logger.h"

template<uint32_t concurrent_connection_count = 1024>
class WebSocket {
private:
	int socket_fd;
	int connection_fds[concurrent_connection_count];

	std::shared_ptr<RingBuffer<std::string>> response_queue;
	std::shared_ptr<RingBuffer<std::string>> request_queue;

	std::optional<std::string> request_buffer;

	void accept_connection(void);
	void read_connections(void);
	void write_connections(void);

	// NOTE(Lukas Karafiat): 64kiByte - 1 Byte for NULL-terminator
	static const size_t maximum_payload_size = USHRT_MAX;
	char payload_buffer[maximum_payload_size];

	std::optional<std::string> receive_message(int);
	void send_message(int, std::string);

public:
	WebSocket(void) = delete;
	// NOTE(Lukas Karafiat): the function declaration got out of hand, had to shorten it quite a bit
	explicit WebSocket(
		std::string,
		std::shared_ptr<RingBuffer<std::string>>,
		std::shared_ptr<RingBuffer<std::string>>);
	~WebSocket(void);

	// non copyable
	WebSocket(const WebSocket &) = delete;
	void operator=(const WebSocket &) = delete;

	// movable
	WebSocket(WebSocket &&) = default;
	WebSocket& operator=(WebSocket &&) = default;

	void run(void);
};


template<uint32_t concurrent_connection_count>
WebSocket<concurrent_connection_count>::WebSocket(
	std::string port,
	std::shared_ptr<RingBuffer<std::string>> response_queue,
	std::shared_ptr<RingBuffer<std::string>> request_queue
) :
	response_queue(response_queue),
	request_queue(request_queue)
{
	// build socket
	struct addrinfo hints = {
		.ai_flags = AI_PASSIVE,
		.ai_family = AF_INET,
		.ai_socktype = SOCK_STREAM,
	};

	int error;

	struct addrinfo *address_info;
	error = getaddrinfo(NULL, port.c_str(), &hints, &address_info);
	if (error != 0) {
		freeaddrinfo(address_info);
		throw std::system_error(errno, std::generic_category(), std::string("could not get address info: ") + gai_strerror(error));
	}

	socket_fd = socket(address_info->ai_family, address_info->ai_socktype | SOCK_NONBLOCK, address_info->ai_protocol);
	if (socket_fd < 0) {
		freeaddrinfo(address_info);
		throw std::system_error(errno, std::generic_category(), "could not create socket");
	}

	int option_value = 1;
	error = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(option_value));
	if (error != 0) {
		close(socket_fd);
		freeaddrinfo(address_info);
		throw std::system_error(errno, std::generic_category(), "could not set socket options");
	}

	error = bind(socket_fd, address_info->ai_addr, address_info->ai_addrlen);
	freeaddrinfo(address_info);
	if (error != 0) {
		close(socket_fd);
		throw std::system_error(errno, std::generic_category(), "could not bind port: " + port);
	}

	error = listen(socket_fd, concurrent_connection_count);
	if (error != 0) {
		close(socket_fd);
		throw std::system_error(errno, std::generic_category(), "could not listen on port: " + port);
	}

	log<INFO>("web server", "listening on port " + port);

	// clear connection_fd buffer
	std::fill_n(connection_fds, concurrent_connection_count, -1);
}


template<uint32_t concurrent_connection_count>
WebSocket<concurrent_connection_count>::~WebSocket(
	void
) {
	close(socket_fd);

	for (uint32_t i = 0; i < concurrent_connection_count; i++) {
		if (0 <= connection_fds[i]) {
			close(connection_fds[i]);
		}
	}

	return;
}


template<uint32_t concurrent_connection_count>
void
WebSocket<concurrent_connection_count>::run(
	void
) {
	extern volatile sig_atomic_t finished;

	while (!finished) {
		accept_connection();
		read_connections();
		write_connections();
	}

	return;
}


template<uint32_t concurrent_connection_count>
void
WebSocket<concurrent_connection_count>::accept_connection(
	void
) {
	int connection_fd = accept(socket_fd, NULL, NULL);
	if (connection_fd < 0 && (errno ==  EAGAIN || errno == EWOULDBLOCK)) {
		return;

	} else if (connection_fd < 0) {
		throw std::system_error(errno, std::generic_category(), "could not accept connection");
	}

	// put connection file descriptor in list
	for (uint32_t i = 0; i < concurrent_connection_count; i++) {
		if (connection_fds[i] < 0) {
			connection_fds[i] = connection_fd;

			log<DEBUG>("web server", "accepted connection at lot " + std::to_string(i));

			break;
		}
	}

	return;
}


template<uint32_t concurrent_connection_count>
void
WebSocket<concurrent_connection_count>::read_connections(
	void
) {
	if (request_buffer.has_value()) {
		bool push_successful = request_queue->push(request_buffer.value());

		if (push_successful) {
			request_buffer.reset();
		}

		return;
	}

	for (uint32_t i = 0; i < concurrent_connection_count; i++) {
		if (connection_fds[i] < 0) {
			continue;
		}

		request_buffer = receive_message(i);

		if (request_buffer.has_value()) {
			log<DEBUG>("web server", "received message: '" + request_buffer.value() + "'");

			bool push_successful = request_queue->push(request_buffer.value());

			if (push_successful) {
				request_buffer.reset();
			}

			return;
		}
	}

	return;
}


template<uint32_t concurrent_connection_count>
std::optional<std::string>
WebSocket<concurrent_connection_count>::receive_message(
	int index
) {
	uint16_t payload_size;

	int error;

	// read header
	error = recv(connection_fds[index], &payload_size, sizeof(payload_size), MSG_DONTWAIT);
	if (error == 0) {
		close(connection_fds[index]);
		connection_fds[index] = -1;

		return std::nullopt;

	} else if (error == -1 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) {
		return std::nullopt;

	} else if (error == -1) {
		throw std::system_error(errno, std::generic_category(), "could not receive data from connection");
	}

	// read payload
	error = recv(connection_fds[index], payload_buffer, payload_size, 0);
	if (error == 0) {
		close(connection_fds[index]);
		connection_fds[index] = -1;

		return std::nullopt;

	} else if (error == -1) {
		throw std::system_error(errno, std::generic_category(), "could not receive data from connection");
	}

	payload_buffer[payload_size] = '\0';

	// NOTE(Lukas Karafiat): string allocation is expensive, could be cleaned up
	return std::make_optional(std::string(payload_buffer));
}


template<uint32_t concurrent_connection_count>
void
WebSocket<concurrent_connection_count>::write_connections(
	void
) {
	std::optional<std::string> response_buffer;

	// NOTE(Lukas Karafiat): we could read more than one response from the
	//     queue, but not infinite as it could result in a life lock
	response_buffer = response_queue->pop();

	if (!response_buffer.has_value()) {
		return;
	}

	for (uint32_t i = 0; i < concurrent_connection_count; i++) {
		if (connection_fds[i] < 0) {
			continue;
		}

		send_message(i, response_buffer.value());
	}

	log<DEBUG>("web server", "sent message: '" + response_buffer.value() + "'");

	return;
}


template<uint32_t concurrent_connection_count>
void
WebSocket<concurrent_connection_count>::send_message(
	int index,
	std::string payload
) {
	if ((size_t) maximum_payload_size < payload.size()) {
		throw std::runtime_error("payload too large");
	}

	uint16_t payload_size = payload.size();

	int error;

	error = send(connection_fds[index], &payload_size, sizeof(payload_size), 0);
	if (error == -1) {
		throw std::system_error(errno, std::generic_category(), "could not send data to connection");
	}

	error = send(connection_fds[index], payload.c_str(), payload.size(), 0);
	if (error == -1 && errno == EINTR) {
		return;

	} else if (error == -1) {
		throw std::system_error(errno, std::generic_category(), "could not send data to connection");
	}

	return;
}

#endif /* WEB_SOCKET_HPP */
