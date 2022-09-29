#ifndef PERIPHERIE_CAN_CHANNEL_TYPE_ROCKET_H
#define PERIPHERIE_CAN_CHANNEL_TYPE_ROCKET_H

#include "peripherie/can/helper.h"
#include "peripherie/can/channel.h"
#include "state.h"

namespace peripherie::can::channel_type {
	class rocket : public channel {
	private:
		enum class variable : uint32_t {
			minimum_chamber_pressure,
			minimum_fuel_pressure,
			minimum_oxygen_pressure,
			holddown_timeout,
			refresh_divider,
			state,
			state_status,
			internal_control,
			abort,
			end_of_flight,

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
			set_rocket_state_request,	// rocket_state_payload
			set_rocket_state_response,	// rocket_state_payload
			get_rocket_state_request,	// -
			get_rocket_state_response,	// rocket_state_payload
			internal_control_request,	// -
			internal_control_response,	// -
			abort_request,			// -
			abort_response,			// -
			end_of_flight_request,		// -
			end_of_flight_response,		// -
			auto_check_request,		// -
			auto_check_response,		// -
		};

		enum class rocket_state {
			pad_idle,
			auto_check,
			ignition_sequence,
			hold_down,
			powered_ascent,
			unpowered_ascent,
			depress,
			abort,
		};

		enum class rocket_state_status {
			write_success,
			failure_write_protected,
			writable,
			write_protected,
		};

	public:
		virtual sensor_buffer command_mapper(can::id const, can::generic_message const);
		virtual std::pair<sensor, size_t> sensor_mapper(can::id const, can::sensor_message const, size_t const);
	};
}

#endif /* PERIPHERIE_CAN_CHANNEL_TYPE_ROCKET_H */
