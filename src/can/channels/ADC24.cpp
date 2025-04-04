//
// Created by Markus on 31.03.21.
//

#include "can/ADC24.h"

const std::vector<std::string> ADC24::states =
        {
            "RefreshDivider",
            "LowerThreshold",
            "UpperThreshold",
            "Calibrate",
            "RequestStatus",
            "ResetAllSettings"
        };

const std::map<std::string, std::vector<double>> ADC24::scalingMap =
        {
            {"RefreshDivider", {1.0, 0.0}},
            {"LowerThreshold", {1.0, 0.0}},
            {"UpperThreshold", {1.0, 0.0}},
        };

const std::map<ADC24_VARIABLES , std::string> ADC24::variableMap =
        {
            {ADC24_REFRESH_DIVIDER, "RefreshDivider"},
            {ADC24_LOWER_THRESHOLD, "LowerThreshold"},
            {ADC24_UPPER_THRESHOLD, "UpperThreshold"},
        };

ADC24::ADC24(uint8_t channelID, std::string channelName, std::vector<double> sensorScaling, Node *parent)
        : Channel("ADC24", channelID, std::move(channelName), sensorScaling, parent, ADC24_DATA_N_BYTES), NonNodeChannel(parent)
{
    commandMap = {
        {"SetRefreshDivider", {std::bind(&ADC24::SetRefreshDivider, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetRefreshDivider", {std::bind(&ADC24::GetRefreshDivider, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetLowerThreshold", {std::bind(&ADC24::SetLowerThreshold, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetLowerThreshold", {std::bind(&ADC24::GetLowerThreshold, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetUpperThreshold", {std::bind(&ADC24::SetUpperThreshold, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetUpperThreshold", {std::bind(&ADC24::GetUpperThreshold, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"RequestCalibrate", {std::bind(&ADC24::RequestCalibrate, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"RequestStatus", {std::bind(&ADC24::RequestStatus, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"RequestResetSettings", {std::bind(&ADC24::RequestResetSettings, this, std::placeholders::_1, std::placeholders::_2), {}}},
    };
}

//---------------------------------------------------------------------------------------//
//-------------------------------GETTER & SETTER Functions-------------------------------//
//---------------------------------------------------------------------------------------//

std::vector<std::string> ADC24::GetStates()
{
    std::vector<std::string> states = ADC24::states;
    for (auto &state : states)
    {
        state = GetStatePrefix() + state;
    }
    return states;
}

//-------------------------------------------------------------------------------//
//-------------------------------RECEIVE Functions-------------------------------//
//-------------------------------------------------------------------------------//

void ADC24::ProcessCANCommand(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp)
{
    try
    {
        switch (canMsg->bit.cmd_id)
        {
            case ADC24_RES_GET_VARIABLE:
            case ADC24_RES_SET_VARIABLE:
                GetSetVariableResponse<ADC24_VARIABLES>(canMsg, canMsgLength, timestamp, variableMap, scalingMap);
                break;
            case ADC24_RES_STATUS:
                StatusResponse(canMsg, canMsgLength, timestamp);
                break;
            case ADC24_RES_RESET_SETTINGS:
                ResetSettingsResponse(canMsg, canMsgLength, timestamp);
                break;
            case ADC24_RES_CALIBRATE:
                CalibrateResponse(canMsg, canMsgLength, timestamp);
                break;
            case ADC24_REQ_RESET_SETTINGS:
            case ADC24_REQ_STATUS:
            case ADC24_REQ_SET_VARIABLE:
            case ADC24_REQ_GET_VARIABLE:
            case ADC24_REQ_CALIBRATE:
                throw std::runtime_error("request message type has been received, major fault in protocol");
                break;
            default:
                throw std::runtime_error("ADC24 specific command with command id not supported: " + std::to_string(canMsg->bit.cmd_id));
        }
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("ADC24 '" + this->channelName + "' - ProcessCANCommand: " + std::string(e.what()));
    }
}

void ADC24::CalibrateResponse(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp)
{
    SetState("Calibrate", 1, timestamp);
}

//----------------------------------------------------------------------------//
//-------------------------------SEND Functions-------------------------------//
//----------------------------------------------------------------------------//

void ADC24::SetLowerThreshold(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("LowerThreshold");

    try
    {
        SetVariable(ADC24_REQ_SET_VARIABLE, parent->GetNodeID(), ADC24_LOWER_THRESHOLD,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("ADC24 - SetLowerThreshold: " + std::string(e.what()));
    }
}

void ADC24::GetLowerThreshold(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(ADC24_REQ_GET_VARIABLE, parent->GetNodeID(), ADC24_LOWER_THRESHOLD,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("ADC24 - GetLowerThreshold: " + std::string(e.what()));
    }
}

void ADC24::SetUpperThreshold(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("UpperThreshold");

    try
    {
        SetVariable(ADC24_REQ_SET_VARIABLE, parent->GetNodeID(), ADC24_UPPER_THRESHOLD,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("ADC24 - SetUpperThreshold: " + std::string(e.what()));
    }
}

void ADC24::GetUpperThreshold(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(ADC24_REQ_GET_VARIABLE, parent->GetNodeID(), ADC24_UPPER_THRESHOLD,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("ADC24 - GetUpperThreshold: " + std::string(e.what()));
    }
}

void ADC24::SetRefreshDivider(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("RefreshDivider");

    try
    {
        SetVariable(ADC24_REQ_SET_VARIABLE, parent->GetNodeID(), ADC24_REFRESH_DIVIDER,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("ADC24 - SetRefreshDivider: " + std::string(e.what()));
    }
}

void ADC24::GetRefreshDivider(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(ADC24_REQ_GET_VARIABLE, parent->GetNodeID(), ADC24_REFRESH_DIVIDER,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("ADC24 - GetRefreshDivider: " + std::string(e.what()));
    }
}

void ADC24::RequestCalibrate(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, parent->GetNodeID(), ADC24_REQ_CALIBRATE, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("ADC24 - RequestCalibrate: " + std::string(e.what()));
    }
}

void ADC24::RequestStatus(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, parent->GetNodeID(), ADC24_REQ_STATUS, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("ADC24 - RequestStatus: " + std::string(e.what()));
    }
}

void ADC24::RequestResetSettings(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, parent->GetNodeID(), ADC24_REQ_RESET_SETTINGS, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("ADC24 - RequestResetSettings: " + std::string(e.what()));
    }
}

void ADC24::RequestCurrentState()
{
    std::vector<double> params;

	GetLowerThreshold(params, false);
	GetUpperThreshold(params, false);
}