#include "peripherie/can/CANSocket.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include <string.h>

#include <string>
#include <system_error>

#include "utility/Logger.h"

struct address_info {
	int family;
	int type;
	int protocol;
};


CANSocket::CANSocket(
	std::string interface_name
) :
	interface_name(interface_name)
{
	struct address_info address_info = {
		.family = AF_CAN,
		.type = SOCK_RAW,
		.protocol = CAN_RAW,
	};

	// build socket
	socket_fd = socket(address_info.family, address_info.type, address_info.protocol);
	if (socket_fd < 0) {
		throw std::system_error(errno, std::generic_category(), "could not create can socket");
	}

	int error;

	// get interface info
	struct ifreq interface_requirements;

	strcpy(interface_requirements.ifr_name, interface_name.c_str());
	error = ioctl(socket_fd, SIOCGIFINDEX, &interface_requirements);
	if (error != 0) {
		close(socket_fd);
		throw std::system_error(errno, std::generic_category(), "could not find interface: '" + interface_name + "'");
	}

	struct sockaddr_can can_socket_address;

	can_socket_address.can_family = AF_CAN;
	can_socket_address.can_ifindex = interface_requirements.ifr_ifindex;

	log<DEBUG>("can socket", "can interface '" + std::string(interface_name) + "' at index " + std::to_string(interface_requirements.ifr_ifindex));

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


int
CANSocket::receive_frame(
	struct peripherie_frame &frame
) {
	int error;

	struct can_frame can_frame;
	error = recv(socket_fd, &can_frame, sizeof(can_frame), MSG_DONTWAIT);
	if (error == -1 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) {
		return -2;

	} else if (error == -1) {
		log<ERROR>("can_socket.send_frame", "could not receive frame from '" + interface_name + "': " + strerror(errno));

		return -1;
	}

	frame.id = frame.can_id;
	frame.protocol = CAN;
	frame.payload_size = frame.can_dlc;
	memcpy(frame.payload, can_frame.data, can_frame.can_dlc);

	return 0;
}


int
CANSocket::send_frame(
	struct peripherie_frame &frame
) {
	if (8 < frame.payload_size) {
		log<ERROR>("can_socket.send_frame", "can protocol dictates maximum payload size of 8 bytes");

		return -1;
	}

	struct can_frame can_frame;

	can_frame.can_id = frame.id;
	can_frame.can_dlc = frame.payload_size;
	memcpy(can_frame.data, frame.payload, frame.payload_size);

	int error;

	error = send(socket_fd, &can_frame, sizeof(can_frame), 0);
	if (error == -1 && errno == EINTR) {
		return -2;

	} else if (error == -1) {
		log<ERROR>("can_socket.send_frame", "could not send frame to '" + interface_name + "': " + strerror(errno));

		return -1;
	}

	return 0;
}

