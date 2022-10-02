#include "peripherie/can/channel_type/imu.h"

#include "utility/Logger.h"

namespace peripherie::can::channel_type {
	sensor_buffer
	imu::command_mapper(
		can::id const id,
		can::generic_message const message
	) {
		sensor_buffer sensor_buffer;

		switch (static_cast<command>(message.command_id)) {
		case command::get_variable_response:
		case command::set_variable_response: {
			set_payload const &payload = *reinterpret_cast<set_payload const *>(&message.data);

			// TODO: findout what type variable::measurement and variable::refresh_divider have

			uint32_t value = payload.value;

			sensor_buffer.first[0] = sensor{ .value=value, .node_id=id.node_id, .channel_id=message.info.channel_id, .variable_id=payload.variable_id };
			sensor_buffer.second = 1;
			break;
		}

		case command::status_response:
			log<WARNING>("can command mapper", "IMU_STATUS_RESPONSE: not implemented");
			break;

		case command::reset_settings_response:
			sensor_buffer.first[0] = sensor{ .value=true, .node_id=id.node_id, .channel_id=message.info.channel_id, .variable_id=(uint32_t)variable::reset_settings };
			sensor_buffer.second = 1;
			break;

		case command::calibrate_response:
			sensor_buffer.first[0] = sensor{ .value=true, .node_id=id.node_id, .channel_id=message.info.channel_id, .variable_id=(uint32_t)variable::calibrate };
			sensor_buffer.second = 1;
			break;

		case command::reset_settings_request:
		case command::status_request:
		case command::set_variable_request:
		case command::get_variable_request:
		case command::calibrate_request:
			log<ERROR>("can command mapper", "request message type has been received, major fault in protocol");
			break;

		default:
			log<ERROR>("can command mapper", "imu specific command with command id not supported: " + std::to_string(message.command_id));
		}

		return sensor_buffer;
	}


	std::pair<sensor, size_t>
	imu::sensor_mapper(
		can::id const id,
		can::sensor_message const message,
		size_t const offset
	) {
		log<ERROR>("imu sensor mapper", "unexpected sensor value");

		return std::make_pair(sensor{}, 12);
	}
}

