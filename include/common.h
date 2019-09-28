//
// Created by Markus on 2019-09-27.
//

#ifndef TXV_ECUI_LLSERVER_COMMON_H

#include <stdint.h>
#include <iostream>
#include <stdexcept>
#include <functional>

#include "Debug.h"
#include "json.hpp"


typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t byte;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

using json = nlohmann::json;

#define TXV_ECUI_LLSERVER_COMMON_H

#endif //TXV_ECUI_LLSERVER_COMMON_H
