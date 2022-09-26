#ifndef PERIPHERIE_CAN_CHANNEL_ADC16_H
#define PERIPHERIE_CAN_CHANNEL_ADC16_H

#include "peripherie/can/helper.h"
#include "state.h"

namespace peripherie::can::channel {
	class adc16 {
	private:
		enum class variable {
			measurement,
			refresh_divider,

			reset_settings
		};

		enum class command {			// payload:
			reset_settings_request,		// -
			reset_settings_response,	// -
			status_request,			// -
			status_response,		// TODO
			set_variable_request,		// set_payload
			set_variable_response,		// set_payload
			get_variable_request,		// get_payload
			get_variable_response,		// set_payload
			calibrate_request,		// -
			calibrate_response,		// -
		};

	public:
		static sensor_buffer command_mapper(can::id const, can::message const);
	};
}

#endif /* PERIPHERIE_CAN_CHANNEL_ADC16_H */
