//
// Created by Markus on 2019-09-28.
//

#ifndef TXV_ECUI_LLSERVER_UTILS_H
#define TXV_ECUI_LLSERVER_UTILS_H

#include <fstream>
#include <sstream>

#include "common.h"

namespace utils
{

int64 toMicros(float val);

std::string loadFile(std::string filePath);
void saveFile(std::string filePath, std::string content);

bool keyExists(const json& j, const std::string& key);

}

#endif //TXV_ECUI_LLSERVER_UTILS_H
