#include "peripherie/can/config.h"

#include "utility/Logger.h"
#include "PayloadTypes.h"

#include "peripherie/can/channel/generic.h"

std::array<std::array<command_mapper, maximum_channel_id>, maximum_node_id>
initialize_command_maps(
	void
) {
	std::array<std::array<command_mapper, maximum_channel_id>, maximum_node_id> command_maps;

	command_maps[0][generic_channel::id] = generic_channel::command_mapper;

	return command_maps;
}

