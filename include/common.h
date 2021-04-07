//
// Created by Markus on 2019-09-27.
//

#ifndef TXV_ECUI_LLSERVER_COMMON_H

#define TXV_ECUI_LLSERVER_COMMON_H

#include <stdint.h>
#include <stdexcept>

#include "Debug.h"

enum class CANResult
{
	SUCCESS,
	ERROR,
	NOT_IMPLEMENTED
};

enum class LLResult
{
    SUCCESS,
    ERROR,
    NOT_IMPLEMENTED
};

#endif //TXV_ECUI_LLSERVER_COMMON_H
