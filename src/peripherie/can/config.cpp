#include "peripherie/can/config.h"

#include "utility/Logger.h"
#include "payload_types.h"

#include "peripherie/can/channel/generic.h"

namespace peripherie::can {
	std::array<std::array<command_mapper, maximum_channel_id>, maximum_node_id>
	initialize_command_maps(
		void
	) {
		std::array<std::array<command_mapper, maximum_channel_id>, maximum_node_id> command_maps;

		command_maps[0][channel::generic::id] = channel::generic::command_mapper;

		return command_maps;
	}
}

