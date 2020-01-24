//
// Created by luis on 1/12/20.
//

#ifndef TXV_ECUI_LLSERVER_CONFIG_H
#define TXV_ECUI_LLSERVER_CONFIG_H

#include <map>
#include <string>
#include <json.hpp>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <variant>

using json = nlohmann::json;

class Config {
private:
    static json data;

    //static std::map<std::string, std::string> data;

    static std::variant<int, double, std::string, bool, json> convertJSONtoType(json object);

public:
    static void Init(std::string filePath = "config.json");

    //getData either with a vector of the data tree (eg: {"ECUI", "hcp"}) or a string with '/' as separators
    //(eg: "ECUI/hcp"). At the end you have to extract the result from the std::variant with std::get<0>(std::variant)
    static std::variant<int, double, std::string, bool, json> getData(std::vector<std::string> keyChain); //each step of the access is a separate vector element
    static std::variant<int, double, std::string, bool, json> getData(std::string keyChain); //each step of the access is separated by a /

    static void print();
};

#endif //TXV_ECUI_LLSERVER_CONFIG_H