//
// Created by chris on 9/3/22.
//

#include "logging/Logger.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>

const std::string severity_names[] = {"TRACE", "DEBUG", "INFO", "WARNING", "ERROR", "FATAL"};


template<Severity severity>
void log(std::string source, std::string msg) {
    // Add timestamp
    auto in_time_t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    auto time = std::put_time(std::localtime(&in_time_t), "%F %T");
    // Build output string
    std::stringstream output;
    output << severity_names[severity] << ":" << source << ":" << time << "\t\t" << msg << std::endl;
    // Log to console
    if (severity <= 2) {
        std::cout << &output;
    } else {
        std::cerr << &output;
    }
    // Log to File
    if (logging_in_file){
        std::ofstream log_file(LOGGING_FILE_PATH, std::ios_base::app | std::ios_base::out);
        log_file << &output;
        log_file.close();
    }
    // Log to Inflix
    if (logging_in_influx){
        //todo: write
    }

}

template<typename Type>
void logSensorValue(std::string key, Type value, std::size_t timestamp);
