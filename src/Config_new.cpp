//
// Created by luis on 1/12/20.
//

#include "Config_new.h"

json Config::data;

Config::Config(std::string fileName) {
    std::ifstream file(fileName);
    std::string rawJSON, line;
    while (getline(file, line)) {
        rawJSON += line;
    }
    data = json::parse(rawJSON);
}

json Config::getData(std::vector<std::string> keyChain) {
    json obj = data;
    while (keyChain.size() > 0) {
        obj = obj[keyChain[0]];
        keyChain.erase(keyChain.begin());
    }
    return obj;
}

json Config::getData(std::string keyChain) {
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
    std::cout << std::setw(4) << data << std::endl;
}