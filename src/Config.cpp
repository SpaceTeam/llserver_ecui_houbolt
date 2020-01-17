//
// Created by luis on 1/12/20.
//

#include "Config.h"
#include "utils.h"
#include "common.h"

json Config::data;

void Config::Init(std::string filePath) {

    std::cerr << "hello5" << std::endl;
    data = json::parse(utils::loadFile(filePath));
    std::cerr << "hello2" << std::endl;
}

std::variant<int, double, std::string, bool, json> Config::getData(std::vector<std::string> keyChain) {
    json obj = data;
    while (keyChain.size() > 0) {
        if (utils::keyExists(obj, keyChain[0]))
        {
            obj = obj[keyChain[0]];
            keyChain.erase(keyChain.begin());
        }
        else
        {
            Debug::error("in Config::getData, no object named %s found", keyChain[0].c_str());
            obj = nullptr;
            break;
        }
    }
    return convertJSONtoType(obj);
}

std::variant<int, double, std::string, bool, json> Config::getData(std::string keyChain) {
    std::vector<std::string> keyVector;
    long unsigned int endPos;
    for (long unsigned int pos = 0; pos != std::string::npos; pos = endPos) {
        endPos = keyChain.find('/', pos + 1);
        keyVector.push_back(keyChain.substr(pos, endPos - pos));
        if (endPos != std::string::npos) {
            endPos++;
        }
    }
    return getData(keyVector);
}

void Config::print() {
    std::cerr << std::setw(4) << data << std::endl;
}

std::variant<int, double, std::string, bool, json> Config::convertJSONtoType(json object) {
    if (object == nullptr)
        return object;
    switch (object.type()) {
        case json::value_t::string:
            return object.get<std::string>();
            break;
        case json::value_t::number_integer:
            return object.get<int>();
            break;
        case json::value_t::number_unsigned:
            return object.get<int>();
            break;
        case json::value_t::number_float:
            return object.get<double>();
            break;
        case json::value_t::boolean:
            return object.get<bool>();
            break;
        default:
            return object;
    }
}
