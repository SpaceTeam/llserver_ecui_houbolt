#ifndef CONFIG_H
#define CONFIG_H

const int minimum_severity = 0;

const bool log_to_console = true;
const bool log_to_file = true;
const bool log_to_influx_db = false;

const std::string log_file_path = "log";

const size_t maximum_state_id = 1024;

enum node_id {
	STUFF_1
};

// NOTE(Lukas Karafiat): just keep in mind that you do not give an alias to two integers
enum node_1_channel_id {
	ROCKET_1,
	PRESSURE_1,
	PNEUMATIC_VALVE_1,
	PNEUMATIC_VALVE_2,
};

enum rocket_id {
	rocket_state,
	rocket_state_status,
	rocket_minimum_chamber_pressure,
	rocket_minimum_fuel_pressure,
	rocket_minimum_ox_pressure,
	rocket_holddown_timeout,
	rocket_internal_control,
	rocket_abort,
	rocket_end_of_flight,
	rocket_auto_check,
	rocket_state_refresh_divider,
	rocket_request_status,
	rocket_reset_all_settings,
};

enum pneumatic_valve_id {
	valve_enabled,
	valve_position,
	valve_target_position,
	valve_threshold,
	valve_hysteresis,
	valve_on_channel_id,
	valve_off_channel_id,
	valve_pos_channel_id,
};

enum state_id {
	pneumatic_valve_1_enabled,
	pneumatic_valve_1_position,
	pneumatic_valve_1_target_position,
};

#endif /* CONFIG_H */
