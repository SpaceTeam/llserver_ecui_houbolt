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
    ADC16(uint8_t channelID, std::string channelName, double sensorScaling, Channel *parent) : Channel(channelID, channelName, sensorScaling, parent, 2) {};

    std::vector<std::string> GetStates() override;
};

#endif //LLSERVER_ECUI_HOUBOLT_ADC16_H
