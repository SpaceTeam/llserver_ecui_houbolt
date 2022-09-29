#ifndef PERIPHERIE_CAN_CHANNEL_TYPE_ADC24_H
#define PERIPHERIE_CAN_CHANNEL_TYPE_ADC24_H

#include "peripherie/can/helper.h"
#include "peripherie/can/channel.h"
#include "state.h"

namespace peripherie::can::channel_type {
	class adc24 : public channel {
	private:
		enum class variable : uint32_t {
			measurement,
			lower_threshold,
			upper_threshold,
			refresh_divider,

			reset_settings,
			status,
			calibrate
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
			calibrate_request,		// -
			calibrate_response,		// -
		};

	public:
		virtual sensor_buffer command_mapper(can::id const, can::generic_message const);
		virtual std::pair<sensor, size_t> sensor_mapper(can::id const, can::sensor_message const, size_t const);
	};
}

#endif /* PERIPHERIE_CAN_CHANNEL_TYPE_ADC24_H */
