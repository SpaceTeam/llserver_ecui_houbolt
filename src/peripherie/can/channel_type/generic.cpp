#include "peripherie/can/channel_type/generic.h"

#include "utility/Logger.h"

namespace peripherie::can::channel_type {
	sensor_buffer
	generic::command_mapper(
		can::id const id,
		can::generic_message const message
	) {
		sensor_buffer sensor_buffer;

		switch (static_cast<command>(message.command_id)) {
		case command::get_variable_response:
		case command::set_variable_response: {
			set_payload const &payload = *reinterpret_cast<set_payload const *>(&message.data);

			// TODO: findout what type variable::state, variable::duty_cycle, variable::frequency, variable::measurement, variable::refresh_divider have


			sensor_buffer.first[0] = sensor{ .value=payload.value, .node_id=id.node_id, .channel_id=message.info.channel_id, .variable_id=payload.variable_id };
			sensor_buffer.second = 1;
			break;
		}

		case command::node_status_response:
			log<WARNING>("can command mapper", "GENERIC_NODE_STATUS_RESPONSE: not implemented");
			break;

		case command::reset_settings_response:
			sensor_buffer.first[0] = sensor{ .value=true, .node_id=id.node_id, .channel_id=message.info.channel_id, .variable_id=(uint32_t)variable::reset_settings };
			sensor_buffer.second = 1;
			break;

		case command::sync_clock_response:
			log<WARNING>("can command mapper", "GENERIC_SYNC_CLOCK_RESPONSE: not implemented");
			break;

		case command::set_variable_request:
		case command::get_variable_request:
		case command::node_status_request:
		case command::data_request:
		case command::speaker_request:
		case command::node_info_request:
		case command::reset_settings_request:
		case command::sync_clock_request:
			log<ERROR>("can command mapper", "request message type has been received, major fault in protocol");
			break;

		default:
			log<ERROR>("can command mapper", "node specific command with command id not supported: " + std::to_string(message.command_id));
		}

		return sensor_buffer;
	}
	

	std::pair<sensor, size_t>
	generic::sensor_mapper(
		can::sensor_data const data,
		size_t const offset
	) {
		log<WARNING>("generic_channel", "sensor mapping: not implemented");

		return std::make_pair<sensor, size_t>(sensor{}, 0);
	}
}

