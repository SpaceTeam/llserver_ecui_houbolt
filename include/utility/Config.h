#pragma once

#include <map>
#include <string>
#include <utility/json.hpp>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <variant>


#define CONFIG_FILE_NAME "config.json"
#define MAPPING_FILE_NAME "mapping.json"


class Config
{
	private:
		static nlohmann::json data;
		static std::string configFilePath;
		static std::string mappingFilePath;

		static std::variant<int, double, std::string, bool, nlohmann::json, std::vector<std::string>> convertJSONtoType(nlohmann::json object);

	public:
		static void Init(std::string configPath);

		//getData either with a vector of the data tree (eg: {"ECUI", "version"}) or a string with '/' as separators
		//(eg: "ECUI/version"). At the end you have to extract the result from the std::variant with std::get<0>(std::variant)
		static std::variant<int, double, std::string, bool, nlohmann::json, std::vector<std::string>> getData(std::vector<std::string> keyChain); //each step of the access is a separate vector element
		static std::variant<int, double, std::string, bool, nlohmann::json, std::vector<std::string>> getData(std::string keyChain); //each step of the access is separated by a /

		static std::string getConfigFilePath();
		static std::string getMappingFilePath();

		static void print();
};
