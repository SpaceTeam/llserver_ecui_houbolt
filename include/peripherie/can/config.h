#ifndef PERIPHERIE_CAN_CONFIG_H
#define PERIPHERIE_CAN_CONFIG_H

#include <stdint.h>

#include <functional>

#include "state.h"
#include "peripherie/can/helper.h"


namespace peripherie::can {
	const uint32_t maximum_node_id = 64;
	const uint32_t maximum_channel_id = 64;
	const uint32_t maximum_variable_id = 256;

	using command_mapper = std::function<sensor_buffer(can::id const, can::message const)>;
	using sensor_mapper = std::function<sensor_buffer(can::id const, can::sensor_message const)>;

	std::array<std::array<command_mapper, maximum_channel_id>, maximum_node_id> initialize_command_maps(void);
}

#endif /* PERIPHERIE_CAN_CONFIG_H */
