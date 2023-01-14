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
	private:
	    static const std::string CONFIG_FILE_NAME;
    	static const std::string MAPPING_FILE_NAME;

		static nlohmann::json data;
		static std::string configFilePath;
		static std::string mappingFilePath;

		static std::variant<int, double, std::string, bool, nlohmann::json, std::vector<std::string>, std::vector<int>> convertJSONtoType(nlohmann::json object);

	public:
		static void Init(std::string configPath);

		nlohmann::json &operator[](const std::string &path);

		//getData either with a vector of the data tree (eg: {"ECUI", "version"}) or a string with '/' as separators
		//(eg: "ECUI/version"). At the end you have to extract the result from the std::variant with std::get<0>(std::variant)
		static std::variant<int, double, std::string, bool, nlohmann::json, std::vector<std::string>, std::vector<int>> getData(std::vector<std::string> keyChain); //each step of the access is a separate vector element
		static std::variant<int, double, std::string, bool, nlohmann::json, std::vector<std::string>, std::vector<int>> getData(std::string keyChain); //each step of the access is separated by a /

		static std::string getConfigFilePath();
		static std::string getMappingFilePath();

		static void print();
};
