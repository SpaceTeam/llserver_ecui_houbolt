#ifndef PERIPHERIE_CAN_NODE_H
#define PERIPHERIE_CAN_NODE_H

#include "state.h"
#include "peripherie/can/helper.h"
#include "peripherie/can/channel.h"

namespace peripherie::can {
	class node {
	private:
	public:
		static uint32_t const maximum_id = 64;

		static sensor_buffer sensor_mapper(can::id const, can::sensor_message const, std::array<can::channel *, can::channel::maximum_id> const);
		static sensor_buffer command_mapper(can::id const, can::generic_message const, std::array<can::channel *, can::channel::maximum_id> const);
	};
}

#endif /* PERIPHERIE_CAN_NODE_H */
