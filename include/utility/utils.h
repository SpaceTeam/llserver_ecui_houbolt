//
// Created by Markus on 2019-09-28.
//

#ifndef TXV_ECUI_LLSERVER_UTILS_H
#define TXV_ECUI_LLSERVER_UTILS_H

#include <fstream>
#include <sstream>

#include "common.h"

#include "json.hpp"

namespace utils
{

    int64_t toMicros(double val);

    std::string loadFile(std::string filePath);
    void saveFile(std::string filePath, std::string content);

    bool keyExists(const nlohmann::json& j, const std::string& key);

    /**
     * current unix timestamp in microseconds
     **/
    uint64_t getCurrentTimestamp();

    void matrixMultiply(std::vector<std::vector<double>> &a, std::vector<std::vector<double>> &b, std::vector<std::vector<double>> &result);

}

#endif //TXV_ECUI_LLSERVER_UTILS_H
