#ifndef WEB_SOCKET_CLIENT_HPP
#define WEB_SOCKET_CLIENT_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <climits>

#include <string>
#include <memory>
#include <optional>

#include "utility/RingBuffer.h"

class WebSocketClient {
private:
	int socket_fd;

	std::shared_ptr<RingBuffer<std::string>> request_queue;
	std::shared_ptr<RingBuffer<std::string>> response_queue;

	std::optional<std::string> request_buffer;

	void accept_connection(void);
	void read_connections(void);
	void write_connections(void);

	// NOTE(Lukas Karafiat): 64kiByte - 1 Byte for NULL-terminator
	static const int maximum_payload_size = USHRT_MAX;
	char payload_buffer[maximum_payload_size];

	std::optional<std::string> receive_message(int index);
	void send_message(int index, std::string message);

public:
	WebSocketClient() = delete;
	explicit WebSocketClient(std::string hostname, std::string port, std::shared_ptr<RingBuffer<std::string>>&request_queue, std::shared_ptr<RingBuffer<std::string>>&response_queue);
	~WebSocketClient();

	// non copyable
	WebSocketClient(WebSocketClient const &) = delete;
	void operator=(WebSocketClient const &x) = delete;

	// movable
	WebSocketClient(WebSocketClient &&) = default;
	WebSocketClient& operator=(WebSocketClient &&x) = default;

	void run(void);
};


WebSocketClient<concurrent_connection_count>::WebSocketClient(
	std::string hostname,
	std::string port,
	std::shared_ptr<RingBuffer<std::string>>&request_queue,
	std::shared_ptr<RingBuffer<std::string>>&response_queue
) :
	request_queue(request_queue),
	response_queue(response_queue)
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

	// clear connection_fd buffer
	std::fill_n(connection_fds, concurrent_connection_count, -1);
}


WebSocketClient<concurrent_connection_count>::~WebSocketClient(
	void
) {
	close(socket_fd);

	for (int i = 0; i < concurrent_connection_count; i++) {
		if (0 <= connection_fds[i]) {
			close(connection_fds[i]);
		}
	}

	return;
}


void
WebSocketClient<concurrent_connection_count>::run(
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


void
WebSocketClient<concurrent_connection_count>::accept_connection(
	void
) {
	int connection_fd = accept(socket_fd, NULL, NULL);
	if (connection_fd < 0 && (errno ==  EAGAIN || errno == EWOULDBLOCK)) {
		return;

	} else if (connection_fd < 0) {
		throw std::system_error(errno, std::generic_category(), "could not accept connection");
	}

	// put conenction file descriptor in list
	for (int i = 0; i < concurrent_connection_count; i++) {
		if (connection_fds[i] < 0) {
			connection_fds[i] = connection_fd;
			break;
		}
	}

	return;
}


void
WebSocketClient<concurrent_connection_count>::read_connections(
	void
) {
	if (request_buffer.has_value()) {
		try {
			request_queue->push(request_buffer.value());
			request_buffer.reset();

		} catch(...) {
			// NOTE(Lukas Karafiat): the stored message could not be pushed onto
			//     the queue so we have to try again later
			return;
		}
	}

	for (int i = 0; i < concurrent_connection_count; i++) {
		if (connection_fds[i] < 0) {
			continue;
		}

		request_buffer = receive_message(i);

		if (request_buffer.has_value()) {
			try {
				request_queue->push(request_buffer.value());
				request_buffer.reset();

			} catch(...) {
				// NOTE(Lukas Karafiat): the read message could not be pushed onto
				//     the queue so we have to try again later
				return;
			}
		}
	}

	return;
}


std::optional<std::string>
WebSocketClient<concurrent_connection_count>::receive_message(
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

	} else if (error == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
		// TODO: find good exception name
		throw std::exception();

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


void
WebSocketClient<concurrent_connection_count>::write_connections(
	void
) {
	std::string message;

	// NOTE(Lukas Karafiat): we could read more than one response from the
	//     queue, but not infinite as it could result in a life lock
	try {
		message = response_queue->pop();

	} catch(...) {
		// NOTE(Lukas Karafiat): could not pop any message of the queue so we
		//     have to try again later
		return;
	}

	for (int i = 0; i < concurrent_connection_count; i++) {
		if (connection_fds[i] < 0) {
			continue;
		}

		send_message(i, message);
	}

	return;
}


void
WebSocketClient<concurrent_connection_count>::send_message(
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
	if (error == -1) {
		throw std::system_error(errno, std::generic_category(), "could not send data to connection");
	}

	return;
}

#endif /* WEB_SOCKET_CLIENT_HPP */
