#ifndef PERIPHERIE_CAN_CONFIG_H
#define PERIPHERIE_CAN_CONFIG_H

#include <stdint.h>

#include <functional>

#include "state.h"
#include "peripherie/can/node.h"
#include "peripherie/can/channel.h"
#include "peripherie/can/helper.h"

namespace peripherie::can {
	std::array<std::array<channel *, channel::maximum_id>, node::maximum_id> initialize_channels(void);
}

#endif /* PERIPHERIE_CAN_CONFIG_H */
