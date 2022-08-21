#ifndef SOCKET_CLIENT_HPP
#define SOCKET_CLIENT_HPP

#include <string>
#include <climits>

class SocketClient {
private:
	int socket_fd;

	// NOTE(Lukas Karafiat): 64kiByte - 1 Byte for NULL-terminator
	int maximum_payload_size = USHRT_MAX;

	char *payload_buffer;

public:
	SocketClient() = delete;
	explicit SocketClient(std::string hostname, std::string port);
	~SocketClient(void);

	std::string receive_message(void);
	void send_message(std::string message);
};

#endif /* SOCKET_CLIENT_HPP */
