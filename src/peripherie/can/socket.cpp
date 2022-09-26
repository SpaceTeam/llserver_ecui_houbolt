#include "peripherie/can/socket.h"

#include <cstdlib>
#include <cstdio>
#include <unistd.h>

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include <string>
#include <string>
#include <system_error>
#include <array>
#include <algorithm>

#include "utility/Logger.h"
#include "peripherie/can/helper.h"
#include "peripherie/can/channel/generic.h"
#include "peripherie/can/config.h"

const auto unix_socket = socket;

namespace peripherie::can {
	struct address_info {
		uint32_t family;
		uint32_t type;
		uint32_t protocol;
	};

	socket::socket(
		std::string interface_name
	) :
		interface_name(interface_name)
	{
		address_info address_info{
			.family = AF_CAN,
			.type = SOCK_RAW,
			.protocol = CAN_RAW_FD_FRAMES
		};

		// build socket
		socket_fd = unix_socket(address_info.family, address_info.type, address_info.protocol);
		if (socket_fd < 0) {
			throw std::system_error(errno, std::generic_category(), "could not create can socket");
		}

		int error;

		// get interface info
		ifreq interface_requirements{};
		std::copy_n(interface_name.begin(), interface_name.length(), interface_requirements.ifr_name);

		error = ioctl(socket_fd, SIOCGIFINDEX, &interface_requirements);
		if (error != 0) {
			close(socket_fd);
			throw std::system_error(errno, std::generic_category(), "could not find interface: '" + interface_name + "'");
		}

		sockaddr_can can_socket_address {
			.can_family = AF_CAN,
			.can_ifindex = interface_requirements.ifr_ifindex,
		};

		log<DEBUG>("can socket", "can interface '" + interface_name + "' at index " + std::to_string(interface_requirements.ifr_ifindex));

		// bind interface to socket
		error = bind(socket_fd, (sockaddr *)&can_socket_address, sizeof(can_socket_address));
		if (error != 0) {
			close(socket_fd);
			throw std::system_error(errno, std::generic_category(), "could not bind can interface: " + interface_name);
		}

		// fill mapping data
		command_maps = initialize_command_maps();

		return;
	}


	socket::~socket(
		void
	) {
		close(socket_fd);

		return;
	}


	std::variant<sensor_buffer, int>
	socket::receive_frame(
		void
	) {
		int error;

		sensor_buffer sensors;
		sensors.second = 0;

		can_frame frame{};
		error = recv(socket_fd, &frame, sizeof(frame), MSG_DONTWAIT);
		if (error == -1 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) {
			return sensors;

		} else if (error == -1 || error != sizeof(can_frame)) {
			return -1;
		}

		{
			if (frame.len < 2) {
				// NOTE(Lukas Karafiat): protocol error
				return -1;
			}

			can::id const &id = *reinterpret_cast<can::id const *>(&frame.can_id);
			can::message const &message = *reinterpret_cast<can::message const *>(&frame.data);

			if (message.info.channel_id == channel::generic::id && static_cast<channel::generic::command>(message.command_id) == channel::generic::command::data_response) {
				// interpret data as sensor data
				can::sensor_message const &sensor_message = *reinterpret_cast<can::sensor_message const *>(&frame.data);

				// call function to interpret sent data
				sensors = sensor_maps[id.node_id](id, sensor_message);

			} else {
				// interpret data as command response
				sensors = command_maps[id.node_id][message.info.channel_id](id, message);
			}
		}

		return sensors;
	}


	std::optional<int>
	socket::send_frame(
		actuator actuator
	) {
		can_frame frame{};

		{
			// NOTE(Lukas Karafiat): abra, kadabra, the integer is now a structure!
			can::id &id = *reinterpret_cast<can::id *>(&frame.can_id);

			id.direction = MASTER2NODE_DIRECTION;
			id.node_id = actuator.node_id;
			id.special_command = STANDARD_SPECIAL_COMMAND;
			id.priority = STANDARD_PRIORITY;

			can::message &message = *reinterpret_cast<can::message *>(&frame.data);

			message.info.channel_id = actuator.channel_id;
			message.info.buffer_type = DIRECT_BUFFER;
			message.command_id = actuator.command_id;

			// NOTE(Lukas Karafiat): the following is necessary to delete type security
			if (std::holds_alternative<get_payload>(actuator.value)) {
				std::copy_n(reinterpret_cast<uint8_t *>(&std::get<get_payload>(actuator.value)), sizeof(get_payload), message.data.begin());
				frame.len = 2 + sizeof(get_payload);

			} else if (std::holds_alternative<set_payload>(actuator.value)) {
				std::copy_n(reinterpret_cast<uint8_t *>(&std::get<set_payload>(actuator.value)), sizeof(set_payload), message.data.begin());
				frame.len = 2 + sizeof(set_payload);

			} else if (std::holds_alternative<servo_move_payload>(actuator.value)) {
				std::copy_n(reinterpret_cast<uint8_t *>(&std::get<servo_move_payload>(actuator.value)), sizeof(servo_move_payload), message.data.begin());
				frame.len = 2 + sizeof(servo_move_payload);

			} else if (std::holds_alternative<rocket_state_payload>(actuator.value)) {
				std::copy_n(reinterpret_cast<uint8_t *>(&std::get<rocket_state_payload>(actuator.value)), sizeof(rocket_state_payload), message.data.begin());
				frame.len = 2 + sizeof(rocket_state_payload);
			}
		}

		int error;

		error = send(socket_fd, &frame, sizeof(frame), 0);
		if (error == -1 && errno == EINTR) {
			return -2;

		} else if (error == -1 || error != sizeof(can_frame)) {
			return -1;
		}

		return std::nullopt;
	}
}

