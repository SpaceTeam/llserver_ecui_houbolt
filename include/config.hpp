#ifndef CONFIG
#define CONFIG

#include <string_view>
#include "utility/severity.hpp"

bool constexpr log_to_console = false;
bool constexpr log_to_file = false;
bool constexpr log_to_influx_db = false;

severity constexpr minimum_severity_level = severity::trace;

std::string_view constexpr log_file_path = "help.log";

size_t constexpr input_message_buffer_capacity = 32;
size_t constexpr output_message_buffer_capacity = 32;

size_t constexpr web_message_buffer_capacity = 32;

#endif /* CONFIG */
