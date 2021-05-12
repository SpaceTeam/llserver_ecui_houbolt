//
// Created by Markus on 10.04.21.
//

#include "can/Channel.h"

#include <algorithm>

#include <can_houbolt/can_cmds.h>

const std::vector<std::string> Channel::states = {};
const std::map<std::string, std::vector<double>> Channel::sensorScalingMap = {};

inline int32_t Channel::ScaleAndConvert(double value, double a, double b)
{
    double result = a * value + b;
    if (result < INT32_MIN || result > INT32_MAX)
    {
        throw std::runtime_error("ScaleAndConvert: result exceeds int32_t value range, actual value: " + std::to_string(result));
    }
    return (int32_t) result;
};

/**
 * assumes pointer has valid data!!!
 * @param valuePtr points to array which includes value data
 * @param valueLength returns number of bytes used for value
 * @param value
 */
void Channel::GetSensorValue(uint8_t *valuePtr, uint8_t &valueLength, double &value)
{
    valueLength = this->typeSize;
    int32_t intValue = 0;
    switch (valueLength)
    {
        case 1:
            intValue = (valuePtr[0]);
            break;
        case 2:
            intValue = (valuePtr[0] << 8 | valuePtr[1]);
            break;
        case 3:
            intValue = (valuePtr[0] << 16 | valuePtr[1] << 8 | valuePtr[2]);
            break;
        case 4:
            intValue = (valuePtr[0] << 24 | valuePtr[1] << 16 | valuePtr[2] << 8 | valuePtr[3]);
            break;
        default:
            throw std::logic_error("Channel - GetSensorValue: channel type has more than 4 bytes\n\tgood luck if this exception happens");
    }

    value = (double)(intValue);

}

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
    std::copy_n(command, commandLength, msg.bit.data.uint8);

    uint32_t msgLength = CAN_MSG_LENGTH(commandLength);

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
