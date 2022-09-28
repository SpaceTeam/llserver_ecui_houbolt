#include "peripherie/can/config.h"

#include "utility/Logger.h"
#include "payload_types.h"

#include "peripherie/can/node.h"

#include "peripherie/can/channel.h"
#include "peripherie/can/channel_type/generic.h"
#include "peripherie/can/channel_type/adc16.h"

namespace peripherie::can {

	channel_type::adc16 adc16_1;

	std::array<std::array<channel *, channel::maximum_id>, node::maximum_id>
	initialize_channels(
		void
	) {
		std::array<std::array<channel *, channel::maximum_id>, node::maximum_id> channels;

		channels[0][0] = &adc16_1;

		return channels;
	}
}

