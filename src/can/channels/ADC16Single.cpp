//
// Created by Markus on 31.03.21.
//

#include "can/ADC16Single.h"
#include "../../../include/StateController.h"

#include <vector>

const std::vector<std::string> ADC16Single::states =
        {
            "Measurement",
            "RefreshDivider",
            "Data",
            "Calibrate",
            "RequestStatus",
            "ResetAllSettings"
        };

const std::map<std::string, std::vector<double>> ADC16Single::scalingMap =
        {
            {"Measurement", {1.0, 0.0}},
            {"RefreshDivider", {1.0, 0.0}},
            {"Data", {1.0, 0.0}},
        };

const std::map<ADC16_SINGLE_VARIABLES , std::string> ADC16Single::variableMap =
        {
            {ADC16_SINGLE_MEASUREMENT, "Measurement"},
            {ADC16_SINGLE_REFRESH_DIVIDER, "RefreshDivider"},
            {ADC16_SINGLE_DATA, "Data"},
        };

ADC16Single::ADC16Single(uint8_t channelID, std::string channelName, std::vector<double> sensorScaling, Node *parent)
        : Channel("ADC16Single", channelID, std::move(channelName), sensorScaling, parent, ADC16_SINGLE_DATA_N_BYTES), NonNodeChannel(parent)
{
    commandMap = {
        {"SetMeasurement", {std::bind(&ADC16Single::SetMeasurement, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetMeasurement", {std::bind(&ADC16Single::GetMeasurement, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetRefreshDivider", {std::bind(&ADC16Single::SetRefreshDivider, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetRefreshDivider", {std::bind(&ADC16Single::GetRefreshDivider, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetData", {std::bind(&ADC16Single::SetData, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetData", {std::bind(&ADC16Single::GetData, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"RequestCalibrate", {std::bind(&ADC16Single::RequestCalibrate, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"RequestStatus", {std::bind(&ADC16Single::RequestStatus, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"RequestResetSettings", {std::bind(&ADC16Single::RequestResetSettings, this, std::placeholders::_1, std::placeholders::_2), {}}},
    };
}

//---------------------------------------------------------------------------------------//
//-------------------------------GETTER & SETTER Functions-------------------------------//
//---------------------------------------------------------------------------------------//

std::vector<std::string> ADC16Single::GetStates()
{
    std::vector<std::string> states = ADC16Single::states;
    for (auto &state : states)
    {
        state = GetStatePrefix() + state;
    }
    return states;
}

//-------------------------------------------------------------------------------//
//-------------------------------RECEIVE Functions-------------------------------//
//-------------------------------------------------------------------------------//

void ADC16Single::ProcessCANCommand(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp)
{
    try
    {
        switch (canMsg->bit.cmd_id)
        {
            case ADC16_SINGLE_RES_GET_VARIABLE:
            case ADC16_SINGLE_RES_SET_VARIABLE:
                GetSetVariableResponse<ADC16_SINGLE_VARIABLES>(canMsg, canMsgLength, timestamp, variableMap, scalingMap);
                break;
            case ADC16_SINGLE_RES_STATUS:
                StatusResponse(canMsg, canMsgLength, timestamp);
                break;
            case ADC16_SINGLE_RES_RESET_SETTINGS:
                ResetSettingsResponse(canMsg, canMsgLength, timestamp);
                break;
            case ADC16_SINGLE_RES_CALIBRATE:
                CalibrateResponse(canMsg, canMsgLength, timestamp);
                break;
            case ADC16_SINGLE_REQ_RESET_SETTINGS:
            case ADC16_SINGLE_REQ_STATUS:
            case ADC16_SINGLE_REQ_SET_VARIABLE:
            case ADC16_SINGLE_REQ_GET_VARIABLE:
            case ADC16_SINGLE_REQ_CALIBRATE:
                throw std::runtime_error("request message type has been received, major fault in protocol");
                break;
            default:
                throw std::runtime_error("ADC16Single specific command with command id not supported: " + std::to_string(canMsg->bit.cmd_id));
        }
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("ADC16Single '" + this->channelName + "' - ProcessCANCommand: " + std::string(e.what()));
    }
}

void ADC16Single::CalibrateResponse(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp)
{
    SetState("Calibrate", 1, timestamp);
}

//----------------------------------------------------------------------------//
//-------------------------------SEND Functions-------------------------------//
//----------------------------------------------------------------------------//

void ADC16Single::SetMeasurement(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("Measurement");

    try
    {
        SetVariable(ADC16_SINGLE_REQ_SET_VARIABLE, parent->GetNodeID(), ADC16_SINGLE_MEASUREMENT,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("ADC16Single - SetMeasurement: " + std::string(e.what()));
    }
}

void ADC16Single::GetMeasurement(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(ADC16_SINGLE_REQ_GET_VARIABLE, parent->GetNodeID(), ADC16_SINGLE_MEASUREMENT,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("ADC16Single - GetMeasurement: " + std::string(e.what()));
    }
}

void ADC16Single::SetRefreshDivider(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("RefreshDivider");

    try
    {
        SetVariable(ADC16_SINGLE_REQ_SET_VARIABLE, parent->GetNodeID(), ADC16_SINGLE_REFRESH_DIVIDER,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("ADC16Single - SetRefreshDivider: " + std::string(e.what()));
    }
}

void ADC16Single::GetRefreshDivider(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(ADC16_SINGLE_REQ_GET_VARIABLE, parent->GetNodeID(), ADC16_SINGLE_REFRESH_DIVIDER,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("ADC16Single - GetRefreshDivider: " + std::string(e.what()));
    }
}

void ADC16Single::SetData(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("Data");

    try
    {
        SetVariable(ADC16_SINGLE_REQ_SET_VARIABLE, parent->GetNodeID(), ADC16_SINGLE_DATA,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("ADC16Single - SetData: " + std::string(e.what()));
    }
}

void ADC16Single::GetData(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(ADC16_SINGLE_REQ_GET_VARIABLE, parent->GetNodeID(), ADC16_SINGLE_DATA,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("ADC16Single - GetData: " + std::string(e.what()));
    }
}

void ADC16Single::RequestCalibrate(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, parent->GetNodeID(), ADC16_SINGLE_REQ_CALIBRATE, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("ADC16Single - RequestCalibrate: " + std::string(e.what()));
    }
}

void ADC16Single::RequestStatus(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, parent->GetNodeID(), ADC16_SINGLE_REQ_STATUS, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("ADC16Single - RequestStatus: " + std::string(e.what()));
    }
}

void ADC16Single::RequestResetSettings(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, parent->GetNodeID(), ADC16_SINGLE_REQ_RESET_SETTINGS, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("ADC16Single - RequestResetSettings: " + std::string(e.what()));
    }
}

void ADC16Single::RequestCurrentState()
{
    std::vector<double> params;

    GetMeasurement(params, false);
	GetRefreshDivider(params, false);
    GetData(params, false);
}