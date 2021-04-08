//
// Created by Markus on 31.03.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_ADC16_H
#define LLSERVER_ECUI_HOUBOLT_ADC16_H

#include "common.h"

#include "can/Channel.h"

#include <map>

class ADC16 : public Channel
{

public:
    using Channel::Channel;

    std::vector<std::string> GetStates() override;
};

#endif //LLSERVER_ECUI_HOUBOLT_ADC16_H
