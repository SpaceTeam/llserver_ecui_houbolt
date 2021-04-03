//
// Created by Markus on 31.03.21.
//

#include "can/ADC24.h"
#include "can_houbolt/commands_table.h"

ADC24::ADC24(uint8_t id) : Channel(id)
{

}

Cmd_t ADC24::ParseCommand(std::string cmd, uint8_t* arguments, size_t argumentLength);
{
    Cmd_Meta_t cmdMeta;
    if (string_to_adc24_cmds.find(cmd) != mymap.end()))
    {
        cmdMeta = string_to_adc24_cmds[cmd];
    }
    else
    {
        Debug::error("invalid command");
        return false;
    }

    return Channel::ParseCommand(cmdMeta, arguments, argumentLength);
}