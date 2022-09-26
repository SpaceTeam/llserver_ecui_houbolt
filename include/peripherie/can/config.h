#ifndef PERIPHERIE_CAN_CONFIG_H
#define PERIPHERIE_CAN_CONFIG_H

#include <stdint.h>

const uint32_t maximum_node_id = 64;
const uint32_t maximum_channel_id = 64;
const uint32_t maximum_variable_id = 256;


#include <functional>
#include "State.h"
#include "peripherie/can/helper.h"

using command_mapper = std::function<sensor_buffer(can_id const, can_message const)>;
using sensor_mapper = std::function<sensor_buffer(can_id const, can_sensor_message const)>;

std::array<std::array<command_mapper, maximum_channel_id>, maximum_node_id> initialize_command_maps(void);

#endif /* PERIPHERIE_CAN_CONFIG_H */
