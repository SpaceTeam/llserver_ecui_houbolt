//
// Created by Markus on 03.09.21.
//

#include "can/Control.h"

const std::vector<std::string> Control::states =
    {
        "Enabled",
        "Target",
        "Threshold",
        "Hysteresis",
        "ActuatorChannelID",
        "SensorChannelID",
        "RefreshDivider",
        "RequestStatus",
        "ResetAllSettings"
    };

const std::map<std::string, std::vector<double>> Control::scalingMap =
    {
        {"Enabled", {1.0, 0.0}},
        {"Target", {0.001189720812, -15.19667413}},
        {"Threshold", {1.0, 0.0}},
        {"Hysteresis", {0.001189720812, -15.19667413}},
        {"ActuatorChannelID", {1.0, 0.0}},
        {"SensorChannelID", {1.0, 0.0}},
        {"RefreshDivider", {1.0, 0.0}},
    };

const std::map<CONTROL_VARIABLES, std::string> Control::variableMap =
    {
        {CONTROL_ENABLED, "Enabled"},
        {CONTROL_TARGET, "Target"},
        {CONTROL_THRESHOLD, "Threshold"},
        {CONTROL_HYSTERESIS, "Hysteresis"},
        {CONTROL_ACTUATOR_CHANNEL_ID, "ActuatorChannelID"},
        {CONTROL_SENSOR_CHANNEL_ID, "SensorChannelID"},
        {CONTROL_REFRESH_DIVIDER, "RefreshDivider"},
    };

Control::Control(uint8_t channelID, std::string channelName, std::vector<double> sensorScaling, Node *parent)
    : Channel("Control", channelID, std::move(channelName), sensorScaling, parent, CONTROL_DATA_N_BYTES), NonNodeChannel(parent)
{
    commandMap = {
        {"SetEnabled", {std::bind(&Control::SetEnabled, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetEnabled", {std::bind(&Control::GetEnabled, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetTarget", {std::bind(&Control::SetTarget, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetTarget", {std::bind(&Control::GetTarget, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetThreshold", {std::bind(&Control::SetThreshold, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetThreshold", {std::bind(&Control::GetThreshold, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetHysteresis", {std::bind(&Control::SetHysteresis, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetHysteresis", {std::bind(&Control::GetHysteresis, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetActuatorChannelID", {std::bind(&Control::SetActuatorChannelID, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetActuatorChannelID", {std::bind(&Control::GetActuatorChannelID, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetSensorChannelID", {std::bind(&Control::SetSensorChannelID, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetSensorChannelID", {std::bind(&Control::GetSensorChannelID, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetRefreshDivider", {std::bind(&Control::SetRefreshDivider, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetRefreshDivider", {std::bind(&Control::GetRefreshDivider, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"RequestStatus", {std::bind(&Control::RequestStatus, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"RequestResetSettings", {std::bind(&Control::RequestResetSettings, this, std::placeholders::_1, std::placeholders::_2), {}}},
    };
}

//---------------------------------------------------------------------------------------//
//-------------------------------GETTER & SETTER Functions-------------------------------//
//---------------------------------------------------------------------------------------//

std::vector<std::string> Control::GetStates()
{
    std::vector<std::string> states = Control::states;
    for (auto &state : states)
    {
        state = GetStatePrefix() + state;
    }
    return states;
}

//-------------------------------------------------------------------------------//
//-------------------------------RECEIVE Functions-------------------------------//
//-------------------------------------------------------------------------------//

void Control::ProcessCANCommand(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp)
{
    try
    {
        switch (canMsg->bit.cmd_id)
        {
        case CONTROL_RES_GET_VARIABLE:
        case CONTROL_RES_SET_VARIABLE:
            GetSetVariableResponse<CONTROL_VARIABLES>(canMsg, canMsgLength, timestamp, variableMap, scalingMap);
            break;
        case CONTROL_RES_STATUS:
            StatusResponse(canMsg, canMsgLength, timestamp);
            break;
        case CONTROL_RES_RESET_SETTINGS:
            ResetSettingsResponse(canMsg, canMsgLength, timestamp);
            break;
        case CONTROL_REQ_RESET_SETTINGS:
        case CONTROL_REQ_STATUS:
        case CONTROL_REQ_SET_VARIABLE:
        case CONTROL_REQ_GET_VARIABLE:
            // TODO: comment out after testing
            // throw std::runtime_error("request message type has been received, major fault in protocol");
            break;
        default:
            throw std::runtime_error("Control specific command with command id not supported: " + std::to_string(canMsg->bit.cmd_id));
        }
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Control '" + this->channelName + "' - ProcessCANCommand: " + std::string(e.what()));
    }
}

//----------------------------------------------------------------------------//
//-------------------------------SEND Functions-------------------------------//
//----------------------------------------------------------------------------//

void Control::SetEnabled(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("Enabled");

    try
    {
        SetVariable(CONTROL_REQ_SET_VARIABLE, parent->GetNodeID(), CONTROL_ENABLED,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Control - SetEnabled: " + std::string(e.what()));
    }
}

void Control::GetEnabled(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(CONTROL_REQ_GET_VARIABLE, parent->GetNodeID(), CONTROL_ENABLED,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Control - GetEnabled: " + std::string(e.what()));
    }
}

void Control::SetTarget(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("Target");

    try
    {
        SetVariable(CONTROL_REQ_SET_VARIABLE, parent->GetNodeID(), CONTROL_TARGET,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Control - SetTarget: " + std::string(e.what()));
    }
}

void Control::GetTarget(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(CONTROL_REQ_GET_VARIABLE, parent->GetNodeID(), CONTROL_TARGET,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Control - GetTarget: " + std::string(e.what()));
    }
}

void Control::SetThreshold(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("Threshold");

    try
    {
        SetVariable(CONTROL_REQ_SET_VARIABLE, parent->GetNodeID(), CONTROL_THRESHOLD,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Control - SetThreshold: " + std::string(e.what()));
    }
}

void Control::GetThreshold(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(CONTROL_REQ_GET_VARIABLE, parent->GetNodeID(), CONTROL_THRESHOLD,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Control - GetThreshold: " + std::string(e.what()));
    }
}

void Control::SetHysteresis(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("Hysteresis");

    try
    {
        SetVariable(CONTROL_REQ_SET_VARIABLE, parent->GetNodeID(), CONTROL_HYSTERESIS,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Control - SetHysteresis: " + std::string(e.what()));
    }
}

void Control::GetHysteresis(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(CONTROL_REQ_GET_VARIABLE, parent->GetNodeID(), CONTROL_HYSTERESIS,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Control - GetHysteresis: " + std::string(e.what()));
    }
}

void Control::SetActuatorChannelID(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("ActuatorChannelID");

    try
    {
        SetVariable(CONTROL_REQ_SET_VARIABLE, parent->GetNodeID(), CONTROL_ACTUATOR_CHANNEL_ID,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Control - SetActuatorChannelID: " + std::string(e.what()));
    }
}

void Control::GetActuatorChannelID(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(CONTROL_REQ_GET_VARIABLE, parent->GetNodeID(), CONTROL_ACTUATOR_CHANNEL_ID,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Control - GetActuatorChannelID: " + std::string(e.what()));
    }
}

void Control::SetSensorChannelID(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("SensorChannelID");

    try
    {
        SetVariable(CONTROL_REQ_SET_VARIABLE, parent->GetNodeID(), CONTROL_SENSOR_CHANNEL_ID,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Control - SetSensorChannelID: " + std::string(e.what()));
    }
}

void Control::GetSensorChannelID(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(CONTROL_REQ_GET_VARIABLE, parent->GetNodeID(), CONTROL_SENSOR_CHANNEL_ID,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Control - GetSensorChannelID: " + std::string(e.what()));
    }
}

void Control::SetRefreshDivider(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("RefreshDivider");

    try
    {
        SetVariable(CONTROL_REQ_SET_VARIABLE, parent->GetNodeID(), CONTROL_REFRESH_DIVIDER,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Control - SetRefreshDivider: " + std::string(e.what()));
    }
}

void Control::GetRefreshDivider(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(CONTROL_REQ_GET_VARIABLE, parent->GetNodeID(), CONTROL_REFRESH_DIVIDER,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Control - GetRefreshDivider: " + std::string(e.what()));
    }
}

void Control::RequestStatus(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, parent->GetNodeID(), CONTROL_REQ_STATUS, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Control - RequestStatus: " + std::string(e.what()));
    }
}

void Control::RequestResetSettings(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, parent->GetNodeID(), CONTROL_REQ_RESET_SETTINGS, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Control - RequestResetSettings: " + std::string(e.what()));
    }
}

void Control::RequestCurrentState()
{
    std::vector<double> params;

    GetEnabled(params, false);
    GetTarget(params, false);
    GetThreshold(params, false);
    GetHysteresis(params, false);
    GetActuatorChannelID(params, false);
    GetSensorChannelID(params, false);
    GetRefreshDivider(params, false);
}