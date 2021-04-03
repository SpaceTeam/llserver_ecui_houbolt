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
    ADC24(uint8_t id);

    ~ADC24();

    Cmd_t ParseCommand(std::string cmd, uint8_t* arguments, size_t argumentLength) override;
};

#endif //LLSERVER_ECUI_HOUBOLT_ADC24_H
