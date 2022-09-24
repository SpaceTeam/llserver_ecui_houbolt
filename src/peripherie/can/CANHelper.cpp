#include "CANHelper.h"

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
