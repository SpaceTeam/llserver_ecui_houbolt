//
// Created by Markus on 31.03.21.
//

#include "can/channels/DigitalOut.h"

const std::vector<std::string> DigitalOut::states =
        {
            "State",
            "DutyCycle",
            "Frequency",
            "RefreshDivider",
            "RequestStatus",
            "Measurement",
            "ResetAllSettings"
        };

const std::map<std::string, std::vector<double>> DigitalOut::scalingMap =
        {
            {"State", {1.0, 0.0}},
            {"DutyCycle", {1.0, 0.0}},
            {"Frequency", {1.0, 0.0}},
            {"Measurement", {1.0, 0.0}},
            {"RefreshDivider", {1.0, 0.0}},
        };

const std::map<DIGITAL_OUT_VARIABLES , std::string> DigitalOut::variableMap =
        {
            {DIGITAL_OUT_STATE, "State"},
            {DIGITAL_OUT_DUTY_CYCLE, "DutyCycle"},
            {DIGITAL_OUT_FREQUENCY, "Frequency"},
            {DIGITAL_OUT_MEASUREMENT, "Measurement"},
            {DIGITAL_OUT_SENSOR_REFRESH_DIVIDER, "RefreshDivider"},
        };

DigitalOut::DigitalOut(uint8_t channelID, std::string channelName, std::vector<double> sensorScaling, Node *parent)
        : Channel("DigitalOut", channelID, std::move(channelName), sensorScaling, parent, DIGITAL_OUT_DATA_N_BYTES), NonNodeChannel(parent)
{
    commandMap = {
        {"SetState", {std::bind(&DigitalOut::SetState, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetState", {std::bind(&DigitalOut::GetState, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetDutyCycle", {std::bind(&DigitalOut::SetDutyCycle, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetDutyCycle", {std::bind(&DigitalOut::GetDutyCycle, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetFrequency", {std::bind(&DigitalOut::SetFrequency, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetFrequency", {std::bind(&DigitalOut::GetFrequency, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetRefreshDivider", {std::bind(&DigitalOut::SetRefreshDivider, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetRefreshDivider", {std::bind(&DigitalOut::GetRefreshDivider, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetMeasurement", {std::bind(&DigitalOut::SetMeasurement, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetMeasurement", {std::bind(&DigitalOut::GetMeasurement, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"RequestStatus", {std::bind(&DigitalOut::RequestStatus, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"RequestResetSettings", {std::bind(&DigitalOut::RequestResetSettings, this, std::placeholders::_1, std::placeholders::_2), {}}},
    };
}

//---------------------------------------------------------------------------------------//
//-------------------------------GETTER & SETTER Functions-------------------------------//
//---------------------------------------------------------------------------------------//

std::vector<std::string> DigitalOut::GetStates()
{
    std::vector<std::string> states = DigitalOut::states;
    for (auto &state : states)
    {
        state = GetStatePrefix() + state;
    }
    return states;
}

//-------------------------------------------------------------------------------//
//-------------------------------RECEIVE Functions-------------------------------//
//-------------------------------------------------------------------------------//

void DigitalOut::ProcessCANCommand(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp)
{
    try
    {
        switch (canMsg->bit.cmd_id)
        {
            case DIGITAL_OUT_RES_GET_VARIABLE:
            case DIGITAL_OUT_RES_SET_VARIABLE:
                GetSetVariableResponse<DIGITAL_OUT_VARIABLES>(canMsg, canMsgLength, timestamp, variableMap, scalingMap);
                break;
            case DIGITAL_OUT_RES_STATUS:
                StatusResponse(canMsg, canMsgLength, timestamp);
                break;
            case DIGITAL_OUT_RES_RESET_SETTINGS:
                ResetSettingsResponse(canMsg, canMsgLength, timestamp);
                break;
            case DIGITAL_OUT_REQ_RESET_SETTINGS:
            case DIGITAL_OUT_REQ_STATUS:
            case DIGITAL_OUT_REQ_SET_VARIABLE:
            case DIGITAL_OUT_REQ_GET_VARIABLE:
                //TODO: comment out after testing
                //throw std::runtime_error("request message type has been received, major fault in protocol");
                break;
            default:
                throw std::runtime_error("DigitalOut specific command with command id not supported: " + std::to_string(canMsg->bit.cmd_id));
        }
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("DigitalOut '" + this->channelName + "' - ProcessCANCommand: " + std::string(e.what()));
    }
}

//----------------------------------------------------------------------------//
//-------------------------------SEND Functions-------------------------------//
//----------------------------------------------------------------------------//

void DigitalOut::SetState(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("State");

    try
    {
        SetVariable(DIGITAL_OUT_REQ_SET_VARIABLE, parent->GetNodeID(), DIGITAL_OUT_STATE,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("DigitalOut - SetState: " + std::string(e.what()));
    }
}

void DigitalOut::GetState(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(DIGITAL_OUT_REQ_GET_VARIABLE, parent->GetNodeID(), DIGITAL_OUT_STATE,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("DigitalOut - GetState: " + std::string(e.what()));
    }
}

void DigitalOut::SetDutyCycle(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("DutyCycle");

    try
    {
        SetVariable(DIGITAL_OUT_REQ_SET_VARIABLE, parent->GetNodeID(), DIGITAL_OUT_DUTY_CYCLE,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("DigitalOut - SetDutyCycle: " + std::string(e.what()));
    }
}

void DigitalOut::GetDutyCycle(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(DIGITAL_OUT_REQ_GET_VARIABLE, parent->GetNodeID(), DIGITAL_OUT_DUTY_CYCLE,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("DigitalOut - GetDutyCycle: " + std::string(e.what()));
    }
}

void DigitalOut::SetFrequency(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("Frequency");

    try
    {
        SetVariable(DIGITAL_OUT_REQ_SET_VARIABLE, parent->GetNodeID(), DIGITAL_OUT_FREQUENCY,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("DigitalOut - SetFrequency: " + std::string(e.what()));
    }
}

void DigitalOut::GetFrequency(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(DIGITAL_OUT_REQ_GET_VARIABLE, parent->GetNodeID(), DIGITAL_OUT_FREQUENCY,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("DigitalOut - GetFrequency: " + std::string(e.what()));
    }
}

void DigitalOut::SetMeasurement(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("Measurement");

    try
    {
        SetVariable(DIGITAL_OUT_REQ_SET_VARIABLE, parent->GetNodeID(), DIGITAL_OUT_MEASUREMENT,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("ADC16 - SetMeasurement: " + std::string(e.what()));
    }
}

void DigitalOut::GetMeasurement(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(DIGITAL_OUT_REQ_GET_VARIABLE, parent->GetNodeID(), DIGITAL_OUT_MEASUREMENT,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("ADC16 - GetMeasurement: " + std::string(e.what()));
    }
}

void DigitalOut::SetRefreshDivider(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("RefreshDivider");

    try
    {
        SetVariable(DIGITAL_OUT_REQ_SET_VARIABLE, parent->GetNodeID(), DIGITAL_OUT_SENSOR_REFRESH_DIVIDER,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("DigitalOut - SetRefreshDivider: " + std::string(e.what()));
    }
}

void DigitalOut::GetRefreshDivider(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(DIGITAL_OUT_REQ_GET_VARIABLE, parent->GetNodeID(), DIGITAL_OUT_SENSOR_REFRESH_DIVIDER,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("DigitalOut - GetRefreshDivider: " + std::string(e.what()));
    }
}

void DigitalOut::RequestStatus(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, parent->GetNodeID(), DIGITAL_OUT_REQ_STATUS, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("DigitalOut - RequestStatus: " + std::string(e.what()));
    }
}

void DigitalOut::RequestResetSettings(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, parent->GetNodeID(), DIGITAL_OUT_REQ_RESET_SETTINGS, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("DigitalOut - RequestResetSettings: " + std::string(e.what()));
    }
}

void DigitalOut::RequestCurrentState()
{
    std::vector<double> params;
    
	GetState(params, false);
	GetDutyCycle(params, false);
	GetFrequency(params, false);
	GetMeasurement(params, false);
	GetRefreshDivider(params, false);
}