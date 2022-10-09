#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <cstddef>

const int minimum_severity = 0;

const bool log_to_console = true;
const bool log_to_file = true;
const bool log_to_influx_db = false;

const std::string log_file_path = "log";

const size_t maximum_state_id = 1024;

const size_t sensor_buffer_capacity = 32;
const size_t actuator_buffer_capacity = 32;

const size_t command_buffer_capacity = 32;
const size_t response_buffer_capacity = 32;
const size_t request_buffer_capacity = 32;

enum class state_name {

};

#endif /* CONFIG_H */
