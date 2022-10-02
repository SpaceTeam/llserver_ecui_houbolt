#ifndef PERIPHERIE_CAN_CHANNEL_TYPE_SERVO_H
#define PERIPHERIE_CAN_CHANNEL_TYPE_SERVO_H

#include "peripherie/can/helper.h"
#include "peripherie/can/channel.h"
#include "state.h"

namespace peripherie::can::channel_type {
	class servo : public channel {
	private:
		enum class variable : uint32_t {
			position,
			target_position,
			target_pressure,
			maximum_speed,
			maximum_acceleration,
			maximum_torque,
			p_parameter,
			i_parameter,
			d_parameter,
			sensor_channel_id,
			position_start_point,
			position_end_point,
			pwm_enabled,
			sensor_refresh_divider,
			pressure_control_enabled,
			position_p_parameter,
			position_i_parameter,
			velocity_p_parameter,
			velocity_i_parameter,
			torque_p_parameter,
			torque_i_parameter,
			pressure_hysteresis,
			filter_enabled,
			filter_alpha,
			position_raw,
			refresh_divider,

			reset_settings,
			status,

			sensor_value,
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

#endif /* PERIPHERIE_CAN_CHANNEL_TYPE_SERVO_H */
