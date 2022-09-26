#ifndef PERIPHERIE_CAN_CHANNEL_GENERIC_H
#define PERIPHERIE_CAN_CHANNEL_GENERIC_H

#include <cstdint>

#include "peripherie/can/helper.h"
#include "state.h"

namespace peripherie::can::channel {
	class generic {
	private:
		enum class variable {
			bus1_voltage,
			bus2_voltage,
			power_voltage,
			power_current,
			uart_enabled,
			refresh_divider,
			refresh_rate,
			logging_enabled,

			reset_all_settings
		};

	public:
		enum class command {		// payload:
			reset_all_settings_request,	// -
			reset_all_settings_response,	// -
			status_request,			// -
			status_response,		// TODO
			set_variable_request,		// set_payload
			set_variable_response,		// set_payload
			get_variable_request,		// get_payload
			get_variable_response,		// set_payload
			sync_clock_request,		// ???
			sync_clock_response,		// ???
			data_request,			// -
			data_response,			// data_payload
			node_info_request,		// -
			node_info_response,		// node_info_payload
			node_status_request,		// -
			node_status_response,		// node_status_payload
			speaker_request,		// speaker_payload
			threshold_request,		// threshold_payload
			flash_clear_request,		// -
			flash_status_response,		// flash_status_payload

			TOTAL_COMMANDS
		};

		static uint32_t const id = 63;

		static sensor_buffer command_mapper(can::id const, can::message const);
	};
}

#endif /* PERIPHERIE_CAN_CHANNEL_GENERIC_H */
