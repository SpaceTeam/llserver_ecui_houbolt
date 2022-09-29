#ifndef STATE_H
#define STATE_H

#include <cstdint>

namespace peripherie {
	enum class type : uint32_t {
		can_socket,
	};
}


#include <variant>
#include "payload_types.h"

struct actuator {
	std::variant<get_payload, set_payload, servo_move_payload, get_rocket_state_payload, set_rocket_state_payload> value;

	uint32_t node_id;
	uint32_t channel_id;
	uint32_t command_id;

	peripherie::type peripherie_type;
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


#include <cstddef>
#include <array>

using sensor_buffer = std::pair<std::array<sensor, 32>, size_t>;
using actuator_buffer = std::pair<std::array<actuator, 32>, size_t>;

#endif /* STATE_H */
