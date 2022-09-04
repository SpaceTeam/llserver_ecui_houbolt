#ifndef LOGGING_H
#define LOGGING_H

#include <string>
#include <chrono>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "config.h"

enum Severity {
	TRACE = 0,
	DEBUG = 1,
	INFO = 2,
	WARNING = 3,
	ERROR = 4,
	FATAL = 5
};

const std::string severity_names[] = { "TRACE", "DEBUG", "INFO", "WARNING", "ERROR", "FATAL" };


template<Severity severity>
void
log(
	std::string source,
	std::string message
) {
	if constexpr (severity < minimum_severity) {
		return;

	} else {
		auto system_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		auto iso_time = std::put_time(std::localtime(&system_time), "%F %T");

		std::stringstream log_message;
		log_message << severity_names[severity] << ":" << source << ":" << iso_time << "\t\t" << message << std::endl;

		if constexpr (log_to_console) {
			if constexpr (severity < Severity::WARNING) {
				std::cout << log_message.str();

			} else {
				std::cerr << log_message.str();
			}
		}

		if constexpr (log_to_file) {
			std::ofstream log_file(log_file_path, std::ios_base::app | std::ios_base::out);
			log_file << log_message.str();
			log_file.close();
		}

		if constexpr (log_to_influx_db) {
			// TODO(Christofer Held): implement this
		}
	}

	return;
}

template<typename Type>
void log_sensor_value(std::string key, Type value, uint64_t timestamp);

#endif /* LOGGING_H */
