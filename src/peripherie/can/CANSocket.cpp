#include "peripherie/can/CANSocket.h"

#include <cstdlib>
#include <cstdio>
#include <unistd.h>

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include <cstring>
#include <string>
#include <system_error>
#include <array>
#include <algorithm>

#include "utility/Logger.h"

#include "peripherie/can/CANHelper.h"

// currently I do not know what the different nodes are for
// specific channel_id's like adc16_1, adc16_2 or rocket
// specific value_id's

struct address_info {
	uint32_t family;
	uint32_t type;
	uint32_t protocol;
};


CANSocket::CANSocket(
	std::string interface_name
) :
	interface_name(interface_name)
{
	struct address_info address_info{
		.family = AF_CAN,
		.type = SOCK_RAW,
		.protocol = CAN_RAW_FD_FRAMES
	};

	// build socket
	socket_fd = socket(address_info.family, address_info.type, address_info.protocol);
	if (socket_fd < 0) {
		throw std::system_error(errno, std::generic_category(), "could not create can socket");
	}

	int error;

	// get interface info
	struct ifreq interface_requirements{};
	strcpy(interface_requirements.ifr_name, interface_name.c_str());

	error = ioctl(socket_fd, SIOCGIFINDEX, &interface_requirements);
	if (error != 0) {
		close(socket_fd);
		throw std::system_error(errno, std::generic_category(), "could not find interface: '" + interface_name + "'");
	}

	struct sockaddr_can can_socket_address {
		.can_family = AF_CAN,
		.can_ifindex = interface_requirements.ifr_ifindex,
	};

	log<DEBUG>("can socket", "can interface '" + interface_name + "' at index " + std::to_string(interface_requirements.ifr_ifindex));

	// bind interface to socket
	error = bind(socket_fd, (struct sockaddr *)&can_socket_address, sizeof(can_socket_address));
	if (error != 0) {
		close(socket_fd);
		throw std::system_error(errno, std::generic_category(), "could not bind can interface: " + interface_name);
	}

	return;
}


CANSocket::~CANSocket(
	void
) {
	close(socket_fd);

	return;
}


std::array<uint8_t, 62>
copy_can_data(
	uint8_t const *data
) {
	std::array<uint8_t, 62> result;

	std::copy_n(data, 62, result.begin());

	return result;
}


std::variant<std::optional<struct sensor>, int>
CANSocket::receive_frame(
	void
) {
	int error;

	struct can_frame frame{};
	error = recv(socket_fd, &frame, sizeof(frame), MSG_DONTWAIT);
	if (error == -1 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) {
		return std::nullopt;

	} else if (error == -1 || error != sizeof(struct can_frame)) {
		return -1;
	}

	struct sensor sensor{};

	// could even return multiple sensor values!!
	// TODO: map frame to sensor
	{
		if (frame.len < 2) {
			// NOTE(Lukas Karafiat): protocol error
			return -1;
		}

		can_id id = (int32_t)frame.can_id;

		can_message message{
			.info = frame.data[0],
			.command_id = frame.data[1],
			.data = copy_can_data(frame.data + 2)
		};

		sensor.node_id = id.node_id;
		sensor.channel_id = message.info.channel_id;
		sensor.command_id = message.command_id;

		//sensor.value_id = ???; // depends on command and sensor
		//sensor.value = ???; // depends on command and sensor and mapping
	}

	return std::make_optional(sensor);
}


std::optional<int>
CANSocket::send_frame(
	struct actuator actuator
) {
	struct can_frame frame{};

	// TODO: map actuator to frame
	{
		size_t node_id = 0;
		size_t channel_id = 0;
		size_t value_id = 0;

		int i = actuator.value * can_state_map[node_id][channel_id][value_id].a + can_state_map[node_id][channel_id][value_id].b;
	}

	int error;

	error = send(socket_fd, &frame, sizeof(frame), 0);
	if (error == -1 && errno == EINTR) {
		return -2;

	} else if (error == -1 || error != sizeof(struct can_frame)) {
		return -1;
	}

	return std::nullopt;
}

