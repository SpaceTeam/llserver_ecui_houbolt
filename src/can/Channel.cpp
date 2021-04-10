//
// Created by Markus on 10.04.21.
//

#include <can_houbolt/can_cmds.h>
#include "can/Channel.h"

const std::vector<std::string> Channel::states = {};
const std::map<std::string, std::vector<double>> Channel::scalingMap = {};

void Channel::SendStandardCommand(uint8_t nodeID, uint8_t cmdID, uint8_t *command, uint32_t commandLength,
                                  uint8_t canBusChannelID, CANDriver *driver, bool testOnly)
{
    Can_MessageId_t canID;
    canID.info.direction = MASTER2NODE_DIRECTION;
    canID.info.node_id = nodeID;
    canID.info.special_cmd = STANDARD_SPECIAL_CMD;
    canID.info.priority = STANDARD_PRIORITY;

    Can_MessageData_t msg;
    msg.bit.info.buffer = DIRECT_BUFFER;
    msg.bit.info.channel_id = this->channelID;
    msg.bit.cmd_id = cmdID;
    msg.bit.data.uint8 = command;

    uint32_t msgLength = sizeof(Can_MessageDataInfo_t) + sizeof(uint8_t) + commandLength;

    if (!testOnly)
    {
        driver->SendCANMessage(canBusChannelID, canID.uint32, msg.uint8, msgLength);
    }
}

void Channel::SetVariable(uint8_t cmdID, uint8_t nodeID, uint8_t variableID, std::vector<double> &scalingParams,
                                 std::vector<double> &params, uint8_t canBusChannelID, CANDriver *driver, bool testOnly)
{
    if (scalingParams.size() != 2)
    {
        throw std::runtime_error("2 scaling parameters expected, but " + std::to_string(params.size()) + " were provided");
    }

    if (params.size() == 1)
    {
        int32_t scaledValue = ScaleAndConvert(params[0], scalingParams[0], scalingParams[1]);
        SetMsg_t setMsg;
        setMsg.variable_id = variableID;
        setMsg.value = scaledValue;
        SendStandardCommand(nodeID, cmdID, (uint8_t *) &setMsg, sizeof(setMsg), canBusChannelID, driver, testOnly);
    }
    else
    {
        throw std::runtime_error("only one parameter expected, but " + std::to_string(params.size()) + " were provided");
    }
}

void Channel::GetVariable(uint8_t cmdID, uint8_t nodeID, uint8_t variableID,
        std::vector<double> &params, uint8_t canBusChannelID, CANDriver *driver, bool testOnly)
{
    if (params.empty())
    {
        GetMsg_t getMsg;
        getMsg.variable_id = variableID;
        SendStandardCommand(nodeID, cmdID, (uint8_t *) &getMsg, sizeof(getMsg), canBusChannelID, driver, testOnly);
    }
    else
    {
        throw std::runtime_error("no parameter expected, but " + std::to_string(params.size()) + " were provided");
    }
}
