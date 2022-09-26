#ifndef PERIPHERIE_CAN_SOCKET_HPP
#define PERIPHERIE_CAN_SOCKET_HPP

#include <string>

#include <functional>
#include <variant>
#include <optional>
#include <array>

#include "state.h"
#include "peripherie/can/helper.h"
#include "config.h"
#include "peripherie/can/config.h"


namespace peripherie::can {
	class socket {
	private:
		std::string interface_name;

		int socket_fd;

		std::array<std::array<command_mapper, maximum_channel_id>, maximum_node_id> command_maps;
		std::array<sensor_mapper, maximum_node_id> sensor_maps;

	public:
		socket(void) = delete;
		explicit socket(std::string interface_name);
		~socket(void);

		// non copyable
		socket(socket const &) = delete;
		void operator=(socket const &x) = delete;

		// movable
		socket(socket &&) = default;
		socket& operator=(socket &&x) = default;

		std::variant<sensor_buffer, int> receive_frame(void);
		std::optional<int> send_frame(actuator const actuator);
	};
}

#endif /* PERIPHERIE_CAN_SOCKET_HPP */