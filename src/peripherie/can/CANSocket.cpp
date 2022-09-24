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

#include "utility/Logger.h"

enum can_direction : uint32_t {
	MASTER2NODE_DIRECTION,
	NODE2MASTER_DIRECTION
};

enum can_priority : uint32_t {
	URGENT_PRIORITY,
	HIGH_PRIORITY,
	STANDARD_PRIORITY,
	LOW_PRIORITY
};

enum can_special_command : uint32_t {
	// direction: master->node
	ABORT_SPECIAL_CMD,
	CLOCK_SYNC_SPECIAL_CMD,
	// direction: node->master
	ERROR_SPECIAL_CMD = CLOCK_SYNC_SPECIAL_CMD,
	// direction: node->master | master->node
	INFO_SPECIAL_CMD,
	STANDARD_SPECIAL_CMD,
};

enum can_buffer_type : uint32_t {
	DIRECT_BUFFER,
	ABSOLUTE_BUFFER,
	RELATIVE_BUFFER,
	RESERVED_BUFFER
};

struct can_id {
	uint32_t direction :1;		// bit:     0 | CAN_MessageDirection_t
	uint32_t node_id :6;		// bit:   1-6 | Node ID: 0 - 63
	uint32_t special_command :2;	// bit:   7-8 | CAN_MessageSpecialCmd_t
	uint32_t priority :2;		// bit:  9-11 | CAN_MessagePriority_t
	uint32_t __reserved :21;	// bit: 11-31 | DO NOT USE! (for now)

	can_id(uint32_t);

	can_id& operator=(uint32_t);
	operator uint32_t(void) const;

	void set(uint32_t);
} __attribute__((__packed__));

can_id::can_id(
	uint32_t i
) {
	this->set(i);
}

can_id::operator uint32_t(
	void
) const {
	return (priority << 9)
		+ (special_command << 7)
		+ (node_id << 1)
		+ (direction << 0);
}

can_id&
can_id::operator=(
	uint32_t i
) {
	this->set(i);
	return *this;
}

void
can_id::set(
	uint32_t i
) {
	priority        = (0x00000600 & i) >> 9;
	special_command = (0x00000180 & i) >> 7;
	node_id         = (0x0000007e & i) >> 1;
	direction       = (0x00000001 & i) >> 0;
}

struct can_message_info {
	uint32_t channel_id :6;		// bit:  0-5 | Channel ID: 0 - 63
	uint32_t buffer_type :2;	// bit:  6-7

	can_message_info(uint8_t i);

	can_message_info& operator=(uint8_t i);
	operator uint8_t(void) const;

	void set(uint8_t i);
} __attribute__((__packed__));

can_message_info::can_message_info(
	uint8_t i
) {
	this->set(i);
}

can_message_info::operator uint8_t(
	void
) const {
	return (buffer_type << 6) + (channel_id << 0);
}

can_message_info&
can_message_info::operator=(
	uint8_t i
) {
	this->set(i);
	return *this;
}

void
can_message_info::set(
	uint8_t i
) {
	channel_id  = (0xc0 & i) >> 6;
	buffer_type = (0x3f & i) >> 0;
}

struct can_message {
	can_message_info info;
	uint8_t command_id;
	std::array<uint8_t, 62> data;
} __attribute__((__packed__));


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
			.data = std::to_array<uint8_t, 62>((uint8_t (&)[62])(frame.data))
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

