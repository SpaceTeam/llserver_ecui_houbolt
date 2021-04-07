//
// Created by Markus on 31.03.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_ADC16_H
#define LLSERVER_ECUI_HOUBOLT_ADC16_H

#include "can/Channel.h"

class ADC16 : public Channel
{
public:
    ADC16(uint8_t channelId, const std::string &channelName, double scaling, Channel *parent);

public:

};

#endif //LLSERVER_ECUI_HOUBOLT_ADC16_H
