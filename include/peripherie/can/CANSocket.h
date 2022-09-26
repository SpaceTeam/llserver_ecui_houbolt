#ifndef PERIPHERIE_CAN_SOCKET_HPP
#define PERIPHERIE_CAN_SOCKET_HPP

#include <string>

#include <functional>
#include <variant>
#include <optional>
#include <array>

#include "State.h"
#include "peripherie/can/helper.h"
#include "config.h"
#include "peripherie/can/config.h"

class CANSocket {
private:
	std::string interface_name;

	int socket_fd;

	std::array<std::array<command_mapper, maximum_channel_id>, maximum_node_id> command_maps;
	std::array<sensor_mapper, maximum_node_id> sensor_maps;

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

	std::variant<sensor_buffer, int> receive_frame(void);
	std::optional<int> send_frame(actuator const actuator);
};

#endif /* PERIPHERIE_CAN_SOCKET_HPP */
