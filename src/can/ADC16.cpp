//
// Created by Markus on 31.03.21.
//

#include "can/ADC16.h"

#include <vector>

static const std::vector<std::string> states = {"REFRESH_DIVIDER"};
static const std::map<std::string, std::function<void(std::vector<double>)>> commandsMap;

std::vector<std::string> ADC16::GetStates()
{

}
