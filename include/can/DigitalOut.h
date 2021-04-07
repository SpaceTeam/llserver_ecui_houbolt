//
// Created by Markus on 31.03.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_DIGITALOUT_H
#define LLSERVER_ECUI_HOUBOLT_DIGITALOUT_H

#include "can/Channel.h"

class DigitalOut : public Channel
{
public:
    DigitalOut(uint8_t channelId, const std::string &channelName, double scaling, Channel *parent);

public:
};

#endif //LLSERVER_ECUI_HOUBOLT_DIGITALOUT_H
