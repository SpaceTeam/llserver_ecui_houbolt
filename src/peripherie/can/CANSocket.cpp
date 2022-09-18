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

#include "utility/Logger.h"

#include <map>

struct address_info {
	int family;
	int type;
	int protocol;
};

struct can_transformation_information {
	int id;
	char name[1024];
};

const std::map<int, struct can_transformation_information> can_transformation_information = {
	{ 0, { .id = 0, .name = "test" } },
};


CANSocket::CANSocket(
	std::string interface_name
) :
	interface_name(interface_name)
{
	struct address_info address_info{
		.family = AF_CAN,
		.type = SOCK_RAW,
		.protocol = CAN_RAW
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

	// TODO: map frame to sensor
	{
		int i;

		i |= frame.data[0] <<  0;
		i |= frame.data[1] <<  8;
		i |= frame.data[2] << 16;
		i |= frame.data[3] << 24;

		sensor.value = i;
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
		int i = std::get<int>(actuator.value);
		frame.data[0] = (uint8_t) (i >>  0);
		frame.data[1] = (uint8_t) (i >>  8);
		frame.data[2] = (uint8_t) (i >> 16);
		frame.data[3] = (uint8_t) (i >> 24);
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

