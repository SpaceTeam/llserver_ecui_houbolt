#ifndef WEB_SOCKET_CLIENT_HPP
#define WEB_SOCKET_CLIENT_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <climits>

#include <string>
#include <memory>
#include <optional>
#include <csignal>

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

	std::optional<std::string> receive_message();
	void send_message(std::string message);

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

#endif /* WEB_SOCKET_CLIENT_HPP */
