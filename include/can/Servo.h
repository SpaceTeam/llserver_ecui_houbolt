//
// Created by Markus on 31.03.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_SERVO_H
#define LLSERVER_ECUI_HOUBOLT_SERVO_H

#include "can/Channel.h"

class Servo : public Channel
{
public:
    Servo(uint8_t channelId, const std::string &channelName, double scaling, Channel *parent);

public:
};

#endif //LLSERVER_ECUI_HOUBOLT_SERVO_H
