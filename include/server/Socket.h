#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <string>
#include <climits>

class Socket {
private:
	FILE *connection;

	// NOTE(Lukas Karafiat): 1 Byte is needed for the NULL-terminator
	int maximum_payload_size = USHRT_MAX - 1;

	char *payload_buffer;

public:
	Socket(std::string, std::string);
	~Socket(void);

	std::string receive(void);
	void send(std::string&);
};

#endif /* SOCKET_HPP */
