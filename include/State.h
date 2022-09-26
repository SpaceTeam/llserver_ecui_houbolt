#ifndef STATE_H
#define STATE_H

enum peripherie_type : uint32_t {
	CAN_SOCKET,
};

#include <variant>
#include <cstddef>
#include "PayloadTypes.h"

struct actuator {
	std::variant<get_payload, set_payload, servo_move_payload, rocket_state_payload> value;

	uint32_t node_id;
	uint32_t channel_id;
	uint32_t command_id;

	uint32_t peripherie_type;
};

struct sensor {
	std::variant<bool, uint32_t, double> value;

	uint32_t node_id;
	uint32_t channel_id;
	uint32_t variable_id;
};

struct state {
	std::variant<bool, uint32_t, double> value;
};

using sensor_buffer = std::pair<std::array<sensor, 32>, size_t>;
using actuator_buffer = std::pair<std::array<actuator, 32>, size_t>;

#endif /* STATE_H */
