#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <string>
#include <climits>

class Socket {
private:
	int socket_fd;

	// NOTE(Lukas Karafiat): 64kiByte - 1 Byte for NULL-terminator
	int maximum_payload_size = USHRT_MAX;

	char *payload_buffer;

public:
	Socket() = delete;
	Socket(std::string, std::string);
	~Socket(void);

	std::string receive_message(void);
	void send_message(std::string);
};

#endif /* SOCKET_HPP */
