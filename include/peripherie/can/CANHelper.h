#ifndef CAN_HELPER_H
#define CAN_HELPER_H

#include <array>

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

#endif /* CAN_HELPER_H */
