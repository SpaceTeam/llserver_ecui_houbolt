//
// Created by luis on 1/12/20.
//

#include "utility/Config.h"
#include "utility/utils.h"
#include "common.h"

nlohmann::json Config::data;

void Config::Init(std::string filePath) {

	try
	{
		data = nlohmann::json::parse(utils::loadFile(filePath));
	}
	catch(const std::exception& e)
	{
		std::cerr << "config file not found" << '\n';
		exit(1);
	}
	
}

std::variant<int, double, std::string, bool, nlohmann::json> Config::getData(std::vector<std::string> keyChain) {
    nlohmann::json obj = data;
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

std::variant<int, double, std::string, bool, nlohmann::json> Config::getData(std::string keyChain) {
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

std::variant<int, double, std::string, bool, nlohmann::json> Config::convertJSONtoType(nlohmann::json object) {
    if (object == nullptr)
        return object;
    switch (object.type()) {
        case nlohmann::json::value_t::string:
            return object.get<std::string>();
            break;
        case nlohmann::json::value_t::number_integer:
            return object.get<int>();
            break;
        case nlohmann::json::value_t::number_unsigned:
            return object.get<int>();
            break;
        case nlohmann::json::value_t::number_float:
            return object.get<double>();
            break;
        case nlohmann::json::value_t::boolean:
            return object.get<bool>();
            break;
        default:
            return object;
    }
}
