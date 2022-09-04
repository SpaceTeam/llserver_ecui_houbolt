#ifndef CONFIG_H
#define CONFIG_H

const int minimum_severity = 0;

const bool log_to_console = true;
const bool log_to_file = true;
const bool log_to_influx_db = false;

const std::string log_file_path = "log";

#endif /* CONFIG_H */
