#pragma once

#include <map>
#include <string>
#include <utility/json.hpp>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <variant>


class Config
{
	protected:
	    static const std::string CONFIG_FILE_NAME;
    	static const std::string MAPPING_FILE_NAME;

		nlohmann::json data;
		std::string configFilePath;
		std::string mappingFilePath;


	public:
		Config() {};
		Config(std::string configPath);

		nlohmann::json &operator[](const std::string &path);

		std::string getConfigFilePath();
		std::string getMappingFilePath();

		void print();
};
