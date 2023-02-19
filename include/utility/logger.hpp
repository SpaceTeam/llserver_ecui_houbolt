#ifndef LOGGING
#define LOGGING

#include <string>
#include <chrono>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>

#include "config.hpp"

#include "utility/severity.hpp"

std::array<std::string_view const, 6> constexpr severity_names =
{
	"TRACE",
	"DEBUG",
	"INFO",
	"WARNING",
	"ERROR",
	"FATAL"
};

template<severity severity_level>
auto
log(const std::string source, const std::string message) -> void
{
	bool constexpr is_logging_active = log_to_console || log_to_file || log_to_influx_db;

	if constexpr (static_cast<size_t>(severity_level) < static_cast<size_t>(minimum_severity_level) || !is_logging_active)
	{
		return;
	}
	else
	{
		auto system_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		auto iso_time = std::put_time(std::localtime(&system_time), "%F %T");

		std::stringstream log_message;
		log_message << severity_names[static_cast<size_t>(severity_level)] << ":" << source << ":" << iso_time << "\t\t" << message << std::endl;

		if constexpr (log_to_console)
		{
			if constexpr (severity_level < severity::warning)
			{
				std::cout << log_message.str();
			}
			else
			{
				std::cerr << log_message.str();
			}
		}

		if constexpr (log_to_file)
		{
			std::ofstream log_file(log_file_path, std::ios_base::app | std::ios_base::out);
			log_file << log_message.str();
			log_file.close();
		}

		if constexpr (log_to_influx_db)
		{
			// TODO(Christofer Held): please implement this
		}
	}

	return;
}

template<typename value_type>
auto
log_sensor_value(std::string key, value_type value, uint64_t timestamp) -> void;

#endif /* LOGGING_H */
