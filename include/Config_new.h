//
// Created by luis on 1/12/20.
//

#ifndef TXV_ECUI_LLSERVER_CONFIG_NEW_H
#define TXV_ECUI_LLSERVER_CONFIG_NEW_H

#include <map>
#include <string>
#include <json.hpp>
#include <iostream>
#include <fstream>
#include <iomanip>

using json = nlohmann::json;

class Config {
private:
    static json data;
    //static std::map<std::string, std::string> data;

public:
    Config(std::string fileName = "config.json");

    json getData(std::vector<std::string> keyChain); //each step of the access is a separate vector element
    json getData(std::string keyChain); //each step of the access is separated by a /

    void print();
};

#endif //TXV_ECUI_LLSERVER_CONFIG_NEW_H
