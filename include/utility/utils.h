//
// Created by Markus on 2019-09-28.
//

#ifndef TXV_ECUI_LLSERVER_UTILS_H
#define TXV_ECUI_LLSERVER_UTILS_H

#include <fstream>

#include "common.h"

#include "json.hpp"

namespace utils
{

    int64_t toMicros(double val);


    bool keyExists(const nlohmann::json& j, const std::string& key);

    /**
     * current unix timestamp in microseconds
     **/
    uint64_t getCurrentTimestamp();

    void matrixMultiply(std::vector<std::vector<double>> &a, std::vector<std::vector<double>> &b, std::vector<std::vector<double>> &result);

    std::string replace(const std::string& str, const std::string& from, const std::string& to);
    bool replaceRef(std::string& str, const std::string& from, const std::string& to);

    std::vector<std::string> split(const std::string& str, const std::string& delimiter);
    std::string merge(std::vector<std::string> strList, const std::string& delimiter);

    uint64_t byteArrayToUInt64BigEndian(uint8_t *data);
    
    void strToWCharPtr( const std::string& str, wchar_t *wCharStrOut);
}

#endif //TXV_ECUI_LLSERVER_UTILS_H
