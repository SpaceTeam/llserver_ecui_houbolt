#include "WebSocketClient.h"

WebSocketClient::WebSocketClient(
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

	socket_fd = socket(address_info->ai_family, address_info->ai_socktype, address_info->ai_protocol);
	if (socket_fd < 0) {
		freeaddrinfo(address_info);
		throw std::system_error(errno, std::generic_category(), "could not create socket");
	}

	error = connect(socket_fd, address_info->ai_addr, address_info->ai_addrlen);
	freeaddrinfo(address_info);
	if (error != 0) {
		close(socket_fd);
		throw std::system_error(errno, std::generic_category(), "could not connect to: " + hostname + ":" + port);
	}

	return;
}


WebSocketClient::~WebSocketClient(
	void
) {
	close(socket_fd);

	return;
}


void
WebSocketClient::run(
	void
) {
	extern volatile sig_atomic_t finished;

	while (!finished && socket_fd != -1) {
		read_connections();
		write_connections();
	}

	return;
}


void
WebSocketClient::read_connections(
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

	request_buffer = receive_message();

	return;
}


std::optional<std::string>
WebSocketClient::receive_message(
	void
) {
	uint16_t payload_size;

	int error;

	// read header
	error = recv(socket_fd, &payload_size, sizeof(payload_size), MSG_DONTWAIT);
	if (error == 0) {
		close(socket_fd);
		socket_fd = -1;

		return std::nullopt;

	} else if (error == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
		// TODO: find good exception name
		throw std::exception();

	} else if (error == -1) {
		throw std::system_error(errno, std::generic_category(), "could not receive data from connection");
	}

	// read payload
	error = recv(socket_fd, payload_buffer, payload_size, 0);
	if (error == 0) {
		close(socket_fd);
		socket_fd = -1;

		return std::nullopt;

	} else if (error == -1) {
		throw std::system_error(errno, std::generic_category(), "could not receive data from connection");
	}

	payload_buffer[payload_size] = '\0';

	// NOTE(Lukas Karafiat): string allocation is expensive, could be cleaned up
	return std::make_optional(std::string(payload_buffer));
}


void
WebSocketClient::write_connections(
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

	if (socket_fd < 0) {
		return;
	}

	send_message(message);

	return;
}


void
WebSocketClient::send_message(
	std::string payload
) {
	if ((size_t) maximum_payload_size < payload.size()) {
		throw std::runtime_error("payload too large");
	}

	uint16_t payload_size = payload.size();

	int error;

	error = send(socket_fd, &payload_size, sizeof(payload_size), 0);
	if (error == -1) {
		throw std::system_error(errno, std::generic_category(), "could not send data to connection");
	}

	error = send(socket_fd, payload.c_str(), payload.size(), 0);
	if (error == -1) {
		throw std::system_error(errno, std::generic_category(), "could not send data to connection");
	}

	return;
}

