#include "peripherie/can/channel_type/adc16_single.h"

#include "utility/Logger.h"

namespace peripherie::can::channel_type {
	sensor_buffer
	adc16_single::command_mapper(
		can::id const id,
		can::generic_message const message
	) {
		sensor_buffer sensor_buffer;

		switch (static_cast<command>(message.command_id)) {
		case command::get_variable_response:
		case command::set_variable_response: {
			set_payload const &payload = *reinterpret_cast<set_payload const *>(&message.data);

			uint32_t value = payload.value;

			sensor_buffer.first[0] = sensor{ .value=value, .node_id=id.node_id, .channel_id=message.info.channel_id, .variable_id=payload.variable_id };
			sensor_buffer.second = 1;
			break;
		}

		case command::status_response:
			log<WARNING>("can command mapper", "ADC16_SINGLE_STATUS_RESPONSE: not implemented");
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
			log<ERROR>("can command mapper", "adc16_single specific command with command id not supported: " + std::to_string(message.command_id));
		}

		return sensor_buffer;
	}


	std::pair<sensor, size_t>
	adc16_single::sensor_mapper(
		can::id const id,
		can::sensor_message const message,
		size_t const offset
	) {
		sensor sensor{};

		uint32_t value = 0;

		value |= message.data[offset + 0] << 0;
		value |= message.data[offset + 1] << 8;

		sensor.value = value;
		sensor.node_id = id.node_id;
		sensor.channel_id = message.info.channel_id;
//		sensor.variable_id = static_cast<int32_t>(variable::measurement);

		return std::make_pair(sensor, 2);
	}
}

