#ifndef WEB_SOCKET_CLIENT_HPP
#define WEB_SOCKET_CLIENT_HPP

#include <string>
#include <climits>

class WebSocketClient {
private:
	int socket_fd;

	// NOTE(Lukas Karafiat): 64kiByte - 1 Byte for NULL-terminator
	int maximum_payload_size = USHRT_MAX;

	char *payload_buffer;

public:
	WebSocketClient() = delete;
	explicit WebSocketClient(std::string hostname, std::string port);
	~WebSocketClient(void);

	std::string receive_message(void);
	void send_message(std::string message);
};

#endif /* WEB_SOCKET_CLIENT_HPP */
