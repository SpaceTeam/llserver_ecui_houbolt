#include "WebSocketClient.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <cstdio>
#include <cerrno>

#include <system_error>
#include <stdexcept>

WebSocketClient::WebSocketClient(
	std::string hostname,
	std::string port
) {
	// build socket
	struct addrinfo hints = {
		.ai_flags = AI_PASSIVE,
		.ai_family = AF_INET,
		.ai_socktype = SOCK_STREAM,
	};

	int error;

	struct addrinfo *address_info;
	error = getaddrinfo(hostname.c_str(), port.c_str(), &hints, &address_info);
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
		throw std::system_error(errno, std::generic_category(), std::string("could not connect to: ") + hostname + ":" + port);
	}

	// allocate payload buffer
	payload_buffer = new char[maximum_payload_size + 1];
	payload_buffer[maximum_payload_size] = '\0';

	return;
}


WebSocketClient::~WebSocketClient(
	void
) {
	close(socket_fd);

	delete payload_buffer;

	return;
}


std::string
WebSocketClient::receive_message(
	void
) {
	uint16_t payload_size;

	int error;

	// read header
	error = recv(socket_fd, &payload_size, sizeof(payload_size), MSG_DONTWAIT);
	if (error == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
		// TODO: find good exception name
		throw std::exception();

	} else if (error == -1) {
		throw std::system_error(errno, std::generic_category(), "could not receive data from connection");
	}

	// read payload
	error = recv(socket_fd, payload_buffer, payload_size, MSG_DONTWAIT);
	if (error == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
		// TODO: find good exception name
		throw std::exception();

	} else if (error == -1) {
		throw std::system_error(errno, std::generic_category(), "could not receive data from connection");
	}

	// NOTE(Lukas Karafiat): string allocation is expensive, could be cleaned up
	return std::string(payload_buffer);
}


void
WebSocketClient::send_message(
	std::string payload
) {
	if ((size_t) maximum_payload_size < payload.size()) {
		throw std::runtime_error("sent payload too large");
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

