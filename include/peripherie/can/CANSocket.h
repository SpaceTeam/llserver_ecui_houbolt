#ifndef PERIPHERIE_CAN_SOCKET_HPP
#define PERIPHERIE_CAN_SOCKET_HPP

#include <string>

#include <optional>
#include <variant>

#include "State.h"

class CANSocket {
private:
	std::string interface_name;

	int socket_fd;

public:
	CANSocket(void) = delete;
	explicit CANSocket(std::string interface_name);
	~CANSocket(void);

	// non copyable
	CANSocket(CANSocket const &) = delete;
	void operator=(CANSocket const &x) = delete;

	// movable
	CANSocket(CANSocket &&) = default;
	CANSocket& operator=(CANSocket &&x) = default;

	std::variant<std::optional<struct sensor>, int> receive_frame(void);
	std::optional<int> send_frame(struct actuator const actuator);
};

#endif /* PERIPHERIE_CAN_SOCKET_HPP */
