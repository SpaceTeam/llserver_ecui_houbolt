#ifndef PERIPHERIE_CAN_HELPER_H
#define PERIPHERIE_CAN_HELPER_H

#include <cstdint>
#include <array>

namespace peripherie::can {
	using message_data = std::array<uint8_t, 62>;
	using sensor_data = std::array<uint8_t, 58>;

	enum direction : uint32_t {
		MASTER2NODE_DIRECTION,
		NODE2MASTER_DIRECTION
	};

	enum priority : uint32_t {
		URGENT_PRIORITY,
		HIGH_PRIORITY,
		STANDARD_PRIORITY,
		LOW_PRIORITY
	};

	enum special_command : uint32_t {
		// direction: master->node
		ABORT_SPECIAL_COMMAND,
		CLOCK_SYNC_SPECIAL_COMMAND,
		// direction: node->master
		ERROR_SPECIAL_COMMAND = CLOCK_SYNC_SPECIAL_COMMAND,
		// direction: node->master | master->node
		INFO_SPECIAL_COMMAND,
		STANDARD_SPECIAL_COMMAND
	};

	enum buffer_type : uint32_t {
		DIRECT_BUFFER,
		ABSOLUTE_BUFFER,
		RELATIVE_BUFFER,
		RESERVED_BUFFER
	};

	struct id {
		uint32_t direction :1;		// bit:     0 | CAN_MessageDirection_t
		uint32_t node_id :6;		// bit:   1-6 | Node ID: 0 - 63
		uint32_t special_command :2;	// bit:   7-8 | CAN_MessageSpecialCmd_t
		uint32_t priority :2;		// bit:  9-11 | CAN_MessagePriority_t
		uint32_t __reserved :21;	// bit: 11-31 | DO NOT USE! (for now)
	} __attribute__((__packed__));

	struct info {
		uint32_t channel_id :6;		// bit:  0-5 | Channel ID: 0 - 63
		uint32_t buffer_type :2;	// bit:  6-7
	} __attribute__((__packed__));

	struct message {
		can::info info;
		uint8_t command_id;
		message_data data;
	} __attribute__((__packed__));

	struct sensor_message {
		can::info info;
		uint8_t command_id;
		uint32_t channel_mask;
		sensor_data data;
	} __attribute__((__packed__));
}

#endif /* PERIPHERIE_CAN_HELPER_H */
