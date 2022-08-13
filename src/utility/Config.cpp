#include "utility/Config.h"
#include "utility/utils.h"
#include "common.h"


nlohmann::json Config::data;
std::string Config::configFilePath = "";
std::string Config::mappingFilePath = "";


void Config::Init(std::string configPath) {
    // SMELL: no Macros !!!
    configFilePath = configPath + "/" + CONFIG_FILE_NAME;
    mappingFilePath = configPath + "/" + MAPPING_FILE_NAME;

    try {
        data = nlohmann::json::parse(utils::loadFile(configFilePath));
    }
    catch (const std::exception &e) {
        std::cerr << "config file parsing failed; " << Config::configFilePath << std::endl;
        std::cerr << e.what() << std::endl;
        exit(1);
    }
}


std::string Config::getConfigFilePath() {
    return configFilePath;
}


std::string Config::getMappingFilePath() {
    return mappingFilePath;
}


std::variant<int, double, std::string, bool, nlohmann::json, std::vector<std::string>>
Config::getData(std::vector<std::string> keyChain) {
    // IMPROVE: this whole function is not needed nlohmann contains this functionality
    nlohmann::json obj = data;
    // SMELL: use empty not size to check if empty
    while (keyChain.size() > 0) {
        // IMPROVE: use contains instead of find
        if (obj.find(keyChain[0]) != obj.end())  {
            obj = obj[keyChain[0]];
            keyChain.erase(keyChain.begin());
        } else {
            Debug::error("in Config::getData, no object named %s found", keyChain[0].c_str());
            obj = nullptr;
            break;
            // SMELL: Exception handling missing what if it breaks here nullptr ist not the best response. use contains?
        }
    }
    return convertJSONtoType(obj);
}

std::variant<int, double, std::string, bool, nlohmann::json, std::vector<std::string>>
Config::getData(std::string keyChain) {
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
    // IMPROVE: this function is never used!
    std::cerr << std::setw(4) << data << std::endl;
}

std::variant<int, double, std::string, bool, nlohmann::json, std::vector<std::string>>
Config::convertJSONtoType(nlohmann::json object) {
    if (object == nullptr)
        return object;
    switch (object.type()) {
        // SMELL: return and break in switch. remove breaks please :-)
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
        case nlohmann::json::value_t::array:
            return object.get<std::vector<std::string>>();
            break;
        default:
            return object;
    }
}
