//
// Created by Markus on 31.03.21.
//

#include "can/channels/DATA32.h"
#include "../../../include/StateController.h"

#include <vector>

const std::vector<std::string> DATA32::states =
        {
            "RefreshDivider",
            "RequestStatus",
            "ResetAllSettings"
        };

const std::map<std::string, std::vector<double>> DATA32::scalingMap =
        {
            {"RefreshDivider", {1.0, 0.0}},
        };

const std::map<DATA32_VARIABLES , std::string> DATA32::variableMap =
        {
            {DATA32_REFRESH_DIVIDER, "RefreshDivider"},
        };

DATA32::DATA32(uint8_t channelID, std::string channelName, std::vector<double> sensorScaling, Node *parent)
        : Channel("DATA32", channelID, std::move(channelName), sensorScaling, parent, DATA32_DATA_N_BYTES), NonNodeChannel(parent)
{
    commandMap = {
        {"SetRefreshDivider", {std::bind(&DATA32::SetRefreshDivider, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetRefreshDivider", {std::bind(&DATA32::GetRefreshDivider, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"RequestStatus", {std::bind(&DATA32::RequestStatus, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"RequestResetSettings", {std::bind(&DATA32::RequestResetSettings, this, std::placeholders::_1, std::placeholders::_2), {}}},
    };
}

//---------------------------------------------------------------------------------------//
//-------------------------------GETTER & SETTER Functions-------------------------------//
//---------------------------------------------------------------------------------------//

std::vector<std::string> DATA32::GetStates()
{
    std::vector<std::string> states = DATA32::states;
    for (auto &state : states)
    {
        state = GetStatePrefix() + state;
    }
    return states;
}

//-------------------------------------------------------------------------------//
//-------------------------------RECEIVE Functions-------------------------------//
//-------------------------------------------------------------------------------//

void DATA32::ProcessCANCommand(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp)
{
    try
    {
        switch (canMsg->bit.cmd_id)
        {
            case DATA32_RES_GET_VARIABLE:
            case DATA32_RES_SET_VARIABLE:
                GetSetVariableResponse<DATA32_VARIABLES>(canMsg, canMsgLength, timestamp, variableMap, scalingMap);
                break;
            case DATA32_RES_STATUS:
                StatusResponse(canMsg, canMsgLength, timestamp);
                break;
            case DATA32_RES_RESET_SETTINGS:
                ResetSettingsResponse(canMsg, canMsgLength, timestamp);
                break;
            case DATA32_REQ_RESET_SETTINGS:
            case DATA32_REQ_STATUS:
            case DATA32_REQ_SET_VARIABLE:
            case DATA32_REQ_GET_VARIABLE:
                throw std::runtime_error("request message type has been received, major fault in protocol");
                break;
            default:
                throw std::runtime_error("DATA32 specific command with command id not supported: " + std::to_string(canMsg->bit.cmd_id));
        }
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("DATA32 '" + this->channelName + "' - ProcessCANCommand: " + std::string(e.what()));
    }
}

//----------------------------------------------------------------------------//
//-------------------------------SEND Functions-------------------------------//
//----------------------------------------------------------------------------//

void DATA32::SetRefreshDivider(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("RefreshDivider");

    try
    {
        SetVariable(DATA32_REQ_SET_VARIABLE, parent->GetNodeID(), DATA32_REFRESH_DIVIDER,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("DATA32 - SetRefreshDivider: " + std::string(e.what()));
    }
}

void DATA32::GetRefreshDivider(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(DATA32_REQ_GET_VARIABLE, parent->GetNodeID(), DATA32_REFRESH_DIVIDER,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("DATA32 - GetRefreshDivider: " + std::string(e.what()));
    }
}

void DATA32::RequestStatus(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, parent->GetNodeID(), DATA32_REQ_STATUS, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("DATA32 - RequestStatus: " + std::string(e.what()));
    }
}

void DATA32::RequestResetSettings(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, parent->GetNodeID(), DATA32_REQ_RESET_SETTINGS, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("DATA32 - RequestResetSettings: " + std::string(e.what()));
    }
}

void DATA32::RequestCurrentState()
{
    std::vector<double> params;

	GetRefreshDivider(params, false);
}