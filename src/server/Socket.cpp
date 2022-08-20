#include "server/Socket.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <cstdio>
#include <cerrno>

#include <system_error>
#include <stdexcept>

Socket::Socket(
	std::string ip,
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
	error = getaddrinfo(ip.c_str(), port.c_str(), &hints, &address_info);
	if (error != 0) {
		freeaddrinfo(address_info);
		throw std::system_error(errno, std::generic_category(), std::string("could not get address info: ") + gai_strerror(error));
	}

	int socket_fd = socket(address_info->ai_family, address_info->ai_socktype, address_info->ai_protocol);
	if (socket_fd < 0) {
		freeaddrinfo(address_info);
		throw std::system_error(errno, std::generic_category(), "could not create socket");
	}

	error = connect(socket_fd, address_info->ai_addr, address_info->ai_addrlen);
	freeaddrinfo(address_info);
	if (error != 0) {
		throw std::system_error(errno, std::generic_category(), std::string("could not connect to: ") + ip + ":" + port);
	}

	connection = fdopen(socket_fd, "rb+");
	if (connection == NULL) {
		close(socket_fd);

		throw std::system_error(errno, std::generic_category(), "could not open connection");
	}

	// allocate payload buffer
	payload_buffer = new char[maximum_payload_size + 1];
	payload_buffer[maximum_payload_size] = '\0';

	return;
}


Socket::~Socket(
	void
) {
	fclose(connection);

	delete payload_buffer;

	return;
}


std::string
Socket::receive(
	void
) {
	uint16_t payload_size;

	// read header
	fread(&payload_size, sizeof(payload_size), 1, connection);

	if (maximum_payload_size < payload_size) {
		throw std::runtime_error("received payload too large");
	}

	// read payload
	fread(payload_buffer, payload_size, 1, connection);

	if (ferror(connection)) {
		throw std::system_error(errno, std::generic_category(), "could not receive data from connection");
	}

	// NOTE(Lukas Karafiat): string allocation is expensive, could be cleaned up
	return std::string(payload_buffer);
}


void
Socket::send(
	std::string payload
) {
	if ((size_t) maximum_payload_size < payload.size()) {
		throw std::runtime_error("sent payload too large");
	}

	uint16_t payload_size = payload.size();

	fwrite(&payload_size, sizeof(payload_size), 1, connection);

	fwrite(payload.c_str(), payload.size(), 1, connection);

	if (ferror(connection)) {
		throw std::system_error(errno, std::generic_category(), "could not send data to connection");
	}

	return;
}

