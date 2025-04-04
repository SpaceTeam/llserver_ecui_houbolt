//
// Created by Markus on 10.04.21.
//

#include "can/channels/Channel.h"

#include <algorithm>

#include "../../../include/can_houbolt/can_cmds.h"

const std::vector<std::string> Channel::states = {};
const std::map<std::string, std::vector<double>> Channel::sensorScalingMap = {};

//---------------------------------------------------------------------------------------//
//-------------------------------GETTER & SETTER Functions-------------------------------//
//---------------------------------------------------------------------------------------//

const std::string Channel::GetChannelTypeName()
{
    return channelTypeName;
}

/**
 * returns of the channel and combines it with channelName to a map
 * @return states of channel, sends a can command for each state, process can command writes the result to the
 * state controller
 */
std::vector<std::string> Channel::GetStates()
{
    std::vector<std::string> states;
    states.insert(states.end(), Channel::states.begin(), Channel::states.end());

    //add node prefix to node specific states
    std::string prefix = GetStatePrefix();
    for (auto &state : states)
    {
        state.insert(0, prefix);
    }

    return states;
};

std::map<std::string, command_t> Channel::GetCommands()
{
    std::map<std::string, command_t> commandsTmp;
    std::map<std::string, command_t> commands;
    commandsTmp.insert(Channel::commandMap.begin(), Channel::commandMap.end());

    //add node prefix to node specific commands
    std::string prefix = GetStatePrefix();
    while (!commandsTmp.empty())
    {
        auto nodeHandle = commandsTmp.extract(commandsTmp.begin()->first);
        nodeHandle.key().insert(0, prefix);
        commands.insert(std::move(nodeHandle));
    }

    return commands;
};

//-------------------------------------------------------------------------------//
//-------------------------------RECEIVE Functions-------------------------------//
//-------------------------------------------------------------------------------//

void Channel::StatusResponse(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp)
{
    throw std::logic_error("Channel - StatusResponse: not implemented");
}

/**
 * requires state to be added to state controller of subclass
 * @param canMsg
 * @param canMsgLength
 * @param timestamp
 */
void Channel::ResetSettingsResponse(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp)
{
    SetState("ResetSettings", 1, timestamp);
}

//----------------------------------------------------------------------------//
//-------------------------------SEND Functions-------------------------------//
//----------------------------------------------------------------------------//

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
            intValue = (valuePtr[1] << 8 | valuePtr[0]);
            break;
        case 3:
            intValue = (valuePtr[2] << 16 | valuePtr[1] << 8 | valuePtr[0]);
            //signed check for 24 bit adc
            if (valuePtr[2]>>7)
            {
                intValue |= 0xFF << 24;
            }
            break;
        case 4:
            intValue = (valuePtr[3] << 24 | valuePtr[2] << 16 | valuePtr[1] << 8 | valuePtr[0]);
            break;
        default:
            throw std::logic_error("Channel - GetSensorValue: channel type has more than 4 bytes\n\tgood luck if this exception happens");
    }
    value = (double)(intValue);

    scalingMtx.lock();
    double slope = sensorScaling[0];
    double offset = sensorScaling[1];
    scalingMtx.unlock();

    value = ScaleSensor(value, slope, offset);
}

void Channel::SendStandardCommand(uint8_t nodeID, uint8_t cmdID, uint8_t *command, uint32_t commandLength,
                                  uint8_t canBusChannelID, CANDriver *driver, bool testOnly)
{
    Can_MessageId_t canID = {0};
    canID.info.direction = MASTER2NODE_DIRECTION;
    canID.info.node_id = nodeID;
    canID.info.special_cmd = STANDARD_SPECIAL_CMD;
    canID.info.priority = STANDARD_PRIORITY;

    Can_MessageData_t msg = {0};
    msg.bit.info.buffer = DIRECT_BUFFER;
    msg.bit.info.channel_id = this->channelID;
    msg.bit.cmd_id = cmdID;
    //TODO: expand to supported data length code for canlib if needed
    if (command != nullptr)
    {
        std::copy_n(command, commandLength, msg.bit.data.uint8);
    }
    else
    {
        if (commandLength != 0)
        {
            throw std::runtime_error("SendStandardCommand: command is nullptr but commandLength != 0");
        }
    }


    uint32_t msgLength = CAN_MSG_LENGTH(commandLength);

    if (!testOnly)
    {
        driver->SendCANMessage(canBusChannelID, canID.uint32, msg.uint8, msgLength, true);
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
        int32_t scaledValue = ScaleAndConvertInt32(params[0], scalingParams[0], scalingParams[1]);
        SetMsg_t setMsg = {0};
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
        GetMsg_t getMsg = {0};
        getMsg.variable_id = variableID;
        Debug::info("GetVariable of channel %s with variable id: %d called", channelName.c_str(), variableID);
        SendStandardCommand(nodeID, cmdID, (uint8_t *) &getMsg, sizeof(getMsg), canBusChannelID, driver, testOnly);
    }
    else
    {
        throw std::runtime_error("no parameter expected, but " + std::to_string(params.size()) + " were provided");
    }
}

void Channel::SendNoPayloadCommand(std::vector<double> &params, uint8_t nodeID, uint8_t cmdID, uint8_t canBusChannelID,
                                   CANDriver *driver, bool testOnly)
{
    if (params.empty())
    {
        SendStandardCommand(nodeID, cmdID, nullptr,
                            0, canBusChannelID, driver, testOnly);
    }
    else
    {
        throw std::runtime_error("no parameter expected, but " + std::to_string(params.size()) + " were provided");
    }
}

//----------------------------------------------------------------------------//
//-----------------------------Utility Functions------------------------------//
//----------------------------------------------------------------------------//

std::vector<double> Channel::ResetSensorOffset(std::vector<double> &params, bool testOnly)
{
    try
    {
        if (params.size() != 1) //number of required parameters
        {
            throw std::runtime_error("1 parameter expected (currValue), but " + std::to_string(params.size()) + " were provided");
        }
        double currValue = params[0];
        scalingMtx.lock();
        sensorScaling[1] = sensorScaling[1] - currValue;
        scalingMtx.unlock();

		return sensorScaling;


    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Channel - ResetSensorOffset: " + std::string(e.what()));
    }
}