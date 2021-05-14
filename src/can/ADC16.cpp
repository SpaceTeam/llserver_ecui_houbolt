//
// Created by Markus on 31.03.21.
//

#include "can/ADC16.h"
#include "StateController.h"

#include <vector>

const std::vector<std::string> ADC16::states =
        {
            "RefreshDivider",
            "Calibrate",
            "RequestStatus",
            "ResetAllSettings"
        };

const std::map<std::string, std::vector<double>> ADC16::scalingMap =
        {
            {"RefreshDivider", {1.0, 0.0}},
        };

const std::map<ADC16_VARIABLES , std::string> ADC16::variableMap =
        {
            {ADC16_REFRESH_DIVIDER, "RefreshDivider"},
        };

ADC16::ADC16(uint8_t channelID, std::string channelName, double sensorScaling, Node *parent)
        : Channel(channelID, std::move(channelName), sensorScaling, parent, ADC16_DATA_N_BYTES), NonNodeChannel(parent)
{
    commandMap = {
        {"SetRefreshDivider", std::bind(&ADC16::SetRefreshDivider, this, std::placeholders::_1, std::placeholders::_2)},
        {"GetRefreshDivider", std::bind(&ADC16::GetRefreshDivider, this, std::placeholders::_1, std::placeholders::_2)},
        {"RequestCalibrate", std::bind(&ADC16::RequestCalibrate, this, std::placeholders::_1, std::placeholders::_2)},
        {"RequestStatus", std::bind(&ADC16::RequestStatus, this, std::placeholders::_1, std::placeholders::_2)},
        {"RequestResetSettings", std::bind(&ADC16::RequestResetSettings, this, std::placeholders::_1, std::placeholders::_2)},
    };
}

//---------------------------------------------------------------------------------------//
//-------------------------------GETTER & SETTER Functions-------------------------------//
//---------------------------------------------------------------------------------------//

std::vector<std::string> ADC16::GetStates()
{
    return ADC16::states;
}

//-------------------------------------------------------------------------------//
//-------------------------------RECEIVE Functions-------------------------------//
//-------------------------------------------------------------------------------//

void ADC16::ProcessCANCommand(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp)
{
    try
    {
        switch (canMsg->bit.cmd_id)
        {
            case ADC16_RES_GET_VARIABLE:
            case ADC16_RES_SET_VARIABLE:
                GetSetVariableResponse<ADC16_VARIABLES>(canMsg, canMsgLength, timestamp, variableMap, scalingMap);
                break;
            case ADC16_RES_STATUS:
                StatusResponse(canMsg, canMsgLength, timestamp);
                break;
            case ADC16_RES_RESET_SETTINGS:
                ResetSettingsResponse(canMsg, canMsgLength, timestamp);
                break;
            case ADC16_RES_CALIBRATE:
                CalibrateResponse(canMsg, canMsgLength, timestamp);
                break;
            case ADC16_REQ_RESET_SETTINGS:
            case ADC16_REQ_STATUS:
            case ADC16_REQ_SET_VARIABLE:
            case ADC16_REQ_GET_VARIABLE:
            case ADC16_REQ_CALIBRATE:
                throw std::runtime_error("request message type has been received, major fault in protocol");
                break;
            default:
                throw std::runtime_error("ADC16 specific command with command id not supported: " + std::to_string(canMsg->bit.cmd_id));
        }
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("ADC16 '" + this->channelName + "' - ProcessCANCommand: " + std::string(e.what()));
    }
}

void ADC16::CalibrateResponse(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp)
{
    SetState("Calibrate", 1, timestamp);
}

//----------------------------------------------------------------------------//
//-------------------------------SEND Functions-------------------------------//
//----------------------------------------------------------------------------//

void ADC16::SetRefreshDivider(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("RefreshDivider");

    try
    {
        SetVariable(ADC16_REQ_SET_VARIABLE, parent->GetNodeID(), ADC16_REFRESH_DIVIDER,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("ADC16 - SetRefreshDivider: " + std::string(e.what()));
    }
}

void ADC16::GetRefreshDivider(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(ADC16_REQ_GET_VARIABLE, parent->GetNodeID(), ADC16_REFRESH_DIVIDER,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("ADC16 - GetRefreshDivider: " + std::string(e.what()));
    }
}

void ADC16::RequestCalibrate(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, parent->GetNodeID(), ADC16_REQ_CALIBRATE, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("ADC16 - RequestCalibrate: " + std::string(e.what()));
    }
}

void ADC16::RequestStatus(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, parent->GetNodeID(), ADC16_REQ_STATUS, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("ADC16 - RequestStatus: " + std::string(e.what()));
    }
}

void ADC16::RequestResetSettings(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, parent->GetNodeID(), ADC16_REQ_RESET_SETTINGS, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("ADC16 - RequestResetSettings: " + std::string(e.what()));
    }
}