#ifndef PERIPHERIE_CAN_CHANNEL_H
#define PERIPHERIE_CAN_CHANNEL_H

#include "peripherie/can/helper.h"
#include "state.h"

namespace peripherie::can {
	class channel {
	private:
	public:
		static uint32_t const maximum_id = 64;

		virtual sensor_buffer command_mapper(can::id const, can::generic_message const) = 0;
		virtual std::pair<sensor, size_t> sensor_mapper(can::id const, can::sensor_message const, size_t const) = 0;
	};
}

#endif /* PERIPHERIE_CAN_CHANNEL_H */
