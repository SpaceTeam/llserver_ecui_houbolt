#include "peripherie/can/node.h"

#include "peripherie/can/config.h"
#include "utility/Logger.h"

#include <tuple>

namespace peripherie::can {
	sensor_buffer
	node::sensor_mapper(
		can::id const id,
		can::sensor_message const message,
		std::array<can::channel *, can::channel::maximum_id> const channels
	) {
		sensor_buffer sensors{};

		size_t offset = 0;
		size_t i = 0;

		for (can::channel_id channel_id = 0; channel_id < channel::maximum_id; channel_id++) {
			if ((0x1 & (message.channel_mask >> channel_id)) == 1) {
				size_t bytes_read;
				std::tie(sensors.first[i], bytes_read) = channels[channel_id]->sensor_mapper(id, message, offset);

				offset += bytes_read;
				i++;
			}
		}

		sensors.second = i;

		return sensors;
	}


	sensor_buffer
	node::command_mapper(
		can::id const id,
		can::generic_message const message,
		std::array<can::channel *, can::channel::maximum_id> const channels
	) {
		sensor_buffer sensors{};

		sensors = channels[message.info.channel_id]->command_mapper(id, message);

		return sensors;
	}
}

