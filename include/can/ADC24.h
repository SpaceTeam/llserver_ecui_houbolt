//
// Created by Markus on 31.03.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_ADC24_H
#define LLSERVER_ECUI_HOUBOLT_ADC24_H

#include "common.h"

#include "can/Channel.h"

class ADC24 : public Channel
{
public:
    ADC24(uint8_t channelID, std::string channelName, double sensorScaling, Channel *parent) : Channel(channelID, channelName, sensorScaling, parent, 3) {};

public:
};

#endif //LLSERVER_ECUI_HOUBOLT_ADC24_H
