#ifndef PERIPHERIE_CAN_CHANNEL_TYPE_PNEUMATIC_VALVE_H
#define PERIPHERIE_CAN_CHANNEL_TYPE_PNEUMATIC_VALVE_H

#include "peripherie/can/helper.h"
#include "peripherie/can/channel.h"
#include "state.h"

namespace peripherie::can::channel_type {
	class pneumatic_valve : public channel {
	private:
		enum class variable : uint32_t {
			enabled,
			position,
			target_position,
			hysteresis,
			on_channel_id,
			off_channel_id,
			position_channel_id,
			position_refresh_divider,

			reset_settings,
			status,
		};

		enum class command : uint32_t {		// payload:
			reset_settings_request,		// -
			reset_settings_response,	// -
			status_request,			// -
			status_response,		// TODO
			set_variable_request,		// set_payload
			set_variable_response,		// set_payload
			get_variable_request,		// get_payload
			get_variable_response,		// set_payload
		};

	public:
		virtual sensor_buffer command_mapper(can::id const, can::generic_message const);
		virtual std::pair<sensor, size_t> sensor_mapper(can::id const, can::sensor_message const, size_t const);
	};
}

#endif /* PERIPHERIE_CAN_CHANNEL_TYPE_PNEUMATIC_VALVE_H */
