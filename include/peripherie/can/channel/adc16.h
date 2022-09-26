#ifndef PERIPHERIE_CAN_CHANNEL_ADC16_H
#define PERIPHERIE_CAN_CHANNEL_ADC16_H

#include "peripherie/can/helper.h"
#include "State.h"

class adc16_channel {
private:
	enum class variable {
		measurement,
		refresh_divider,
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
	static sensor_buffer command_mapper(can_id const, can_message const);
};

#endif /* PERIPHERIE_CAN_CHANNEL_ADC16_H */
