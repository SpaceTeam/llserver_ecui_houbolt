//
// Created by Markus on 03.09.21.
//

#include "can/channels/PIControl.h"

const std::vector<std::string> PIControl::states =
    {
        "Enabled",
        "Target",
        "P_POS",
        "I_POS",
        "P_NEG",
        "I_NEG",
        "SensorSlope",
        "SensorOffset",
        "OperatingPoint",
        "ActuatorChannelID",
        "SensorChannelID",
        "RefreshDivider",
        "RequestStatus",
        "ResetAllSettings"
    };

const std::map<std::string, std::vector<double>> PIControl::scalingMap =
    {
        {"Enabled", {1.0, 0.0}},
        {"Target", {0.001, 0.0}},
        {"P_POS", {0.001, 0.0}},
        {"I_POS", {0.001, 0.0}},
        {"P_NEG", {0.001, 0.0}},
        {"I_NEG", {0.001, 0.0}},
        {"SensorSlope", {0.001, 0.0}},
        {"SensorOffset", {0.001, 0.0}},
        {"OperatingPoint", {0.001, 0.0}},
        {"ActuatorChannelID", {1.0, 0.0}},
        {"SensorChannelID", {1.0, 0.0}},
        {"RefreshDivider", {1.0, 0.0}},
    };

const std::map<PI_CONTROL_VARIABLES, std::string> PIControl::variableMap =
    {
        {PI_CONTROL_ENABLED, "Enabled"},
        {PI_CONTROL_TARGET, "Target"},
        {PI_CONTROL_P_POS, "P_POS"},
        {PI_CONTROL_I_POS, "I_POS"},
        {PI_CONTROL_P_NEG, "P_NEG"},
        {PI_CONTROL_I_NEG, "I_NEG"},
        {PI_CONTROL_SENSOR_SLOPE, "SensorSlope"},
        {PI_CONTROL_SENSOR_OFFSET, "SensorOffset"},
        {PI_CONTROL_OPERATING_POINT, "OperatingPoint"},
        {PI_CONTROL_ACTUATOR_CHANNEL_ID, "ActuatorChannelID"},
        {PI_CONTROL_SENSOR_CHANNEL_ID, "SensorChannelID"},
        {PI_CONTROL_REFRESH_DIVIDER, "RefreshDivider"},
    };

PIControl::PIControl(uint8_t channelID, std::string channelName, std::vector<double> sensorScaling, Node *parent)
    : Channel("PIControl", channelID, std::move(channelName), sensorScaling, parent, PI_CONTROL_DATA_N_BYTES), NonNodeChannel(parent)
{
    commandMap = {
        {"SetEnabled", {std::bind(&PIControl::SetEnabled, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetEnabled", {std::bind(&PIControl::GetEnabled, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetTarget", {std::bind(&PIControl::SetTarget, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetTarget", {std::bind(&PIControl::GetTarget, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetP_POS", {std::bind(&PIControl::SetP_POS, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetP_POS", {std::bind(&PIControl::GetP_POS, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetI_POS", {std::bind(&PIControl::SetI_POS, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetI_POS", {std::bind(&PIControl::GetI_POS, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetP_NEG", {std::bind(&PIControl::SetP_NEG, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetP_NEG", {std::bind(&PIControl::GetP_NEG, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetI_NEG", {std::bind(&PIControl::SetI_NEG, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetI_NEG", {std::bind(&PIControl::GetI_NEG, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetSensorSlope", {std::bind(&PIControl::SetSensorSlope, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetSensorSlope", {std::bind(&PIControl::GetSensorSlope, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetSensorOffset", {std::bind(&PIControl::SetSensorOffset, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetSensorOffset", {std::bind(&PIControl::GetSensorOffset, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetOperatingPoint", {std::bind(&PIControl::SetOperatingPoint, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetOperatingPoint", {std::bind(&PIControl::GetOperatingPoint, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetActuatorChannelID", {std::bind(&PIControl::SetActuatorChannelID, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetActuatorChannelID", {std::bind(&PIControl::GetActuatorChannelID, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetSensorChannelID", {std::bind(&PIControl::SetSensorChannelID, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetSensorChannelID", {std::bind(&PIControl::GetSensorChannelID, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetRefreshDivider", {std::bind(&PIControl::SetRefreshDivider, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetRefreshDivider", {std::bind(&PIControl::GetRefreshDivider, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"RequestStatus", {std::bind(&PIControl::RequestStatus, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"RequestResetSettings", {std::bind(&PIControl::RequestResetSettings, this, std::placeholders::_1, std::placeholders::_2), {}}},
    };
}

//---------------------------------------------------------------------------------------//
//-------------------------------GETTER & SETTER Functions-------------------------------//
//---------------------------------------------------------------------------------------//

std::vector<std::string> PIControl::GetStates()
{
    std::vector<std::string> states = PIControl::states;
    for (auto &state : states)
    {
        state = GetStatePrefix() + state;
    }
    return states;
}

//-------------------------------------------------------------------------------//
//-------------------------------RECEIVE Functions-------------------------------//
//-------------------------------------------------------------------------------//

void PIControl::ProcessCANCommand(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp)
{
    try
    {
        switch (canMsg->bit.cmd_id)
        {
        case PI_CONTROL_RES_GET_VARIABLE:
        case PI_CONTROL_RES_SET_VARIABLE:
            GetSetVariableResponse<PI_CONTROL_VARIABLES>(canMsg, canMsgLength, timestamp, variableMap, scalingMap);
            break;
        case PI_CONTROL_RES_STATUS:
            StatusResponse(canMsg, canMsgLength, timestamp);
            break;
        case PI_CONTROL_RES_RESET_SETTINGS:
            ResetSettingsResponse(canMsg, canMsgLength, timestamp);
            break;
        case PI_CONTROL_REQ_RESET_SETTINGS:
        case PI_CONTROL_REQ_STATUS:
        case PI_CONTROL_REQ_SET_VARIABLE:
        case PI_CONTROL_REQ_GET_VARIABLE:
            // TODO: comment out after testing
            // throw std::runtime_error("request message type has been received, major fault in protocol");
            break;
        default:
            throw std::runtime_error("PIControl specific command with command id not supported: " + std::to_string(canMsg->bit.cmd_id));
        }
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PIControl '" + this->channelName + "' - ProcessCANCommand: " + std::string(e.what()));
    }
}

//----------------------------------------------------------------------------//
//-------------------------------SEND Functions-------------------------------//
//----------------------------------------------------------------------------//

void PIControl::SetEnabled(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("Enabled");

    try
    {
        SetVariable(PI_CONTROL_REQ_SET_VARIABLE, parent->GetNodeID(), PI_CONTROL_ENABLED,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PIControl - SetEnabled: " + std::string(e.what()));
    }
}

void PIControl::GetEnabled(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(PI_CONTROL_REQ_GET_VARIABLE, parent->GetNodeID(), PI_CONTROL_ENABLED,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PIControl - GetEnabled: " + std::string(e.what()));
    }
}

void PIControl::SetTarget(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("Target");

    try
    {
        SetVariable(PI_CONTROL_REQ_SET_VARIABLE, parent->GetNodeID(), PI_CONTROL_TARGET,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PIControl - SetTarget: " + std::string(e.what()));
    }
}

void PIControl::GetTarget(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(PI_CONTROL_REQ_GET_VARIABLE, parent->GetNodeID(), PI_CONTROL_TARGET,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PIControl - GetTarget: " + std::string(e.what()));
    }
}

void PIControl::SetP_POS(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("P_POS");

    try
    {
        SetVariable(PI_CONTROL_REQ_SET_VARIABLE, parent->GetNodeID(), PI_CONTROL_P_POS,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PIControl - SetP_POS: " + std::string(e.what()));
    }
}

void PIControl::GetP_POS(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(PI_CONTROL_REQ_GET_VARIABLE, parent->GetNodeID(), PI_CONTROL_P_POS,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PIControl - GetP_POS: " + std::string(e.what()));
    }
}

void PIControl::SetI_POS(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("I_POS");

    try
    {
        SetVariable(PI_CONTROL_REQ_SET_VARIABLE, parent->GetNodeID(), PI_CONTROL_I_POS,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PIControl - SetI_POS: " + std::string(e.what()));
    }
}

void PIControl::GetI_POS(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(PI_CONTROL_REQ_GET_VARIABLE, parent->GetNodeID(), PI_CONTROL_I_POS,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PIControl - GetI_POS: " + std::string(e.what()));
    }
}

void PIControl::SetP_NEG(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("P_NEG");

    try
    {
        SetVariable(PI_CONTROL_REQ_SET_VARIABLE, parent->GetNodeID(), PI_CONTROL_P_NEG,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PIControl - SetP_NEG: " + std::string(e.what()));
    }
}

void PIControl::GetP_NEG(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(PI_CONTROL_REQ_GET_VARIABLE, parent->GetNodeID(), PI_CONTROL_P_NEG,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PIControl - GetP_NEG: " + std::string(e.what()));
    }
}

void PIControl::SetI_NEG(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("I_NEG");

    try
    {
        SetVariable(PI_CONTROL_REQ_SET_VARIABLE, parent->GetNodeID(), PI_CONTROL_I_NEG,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PIControl - SetI_NEG: " + std::string(e.what()));
    }
}

void PIControl::GetI_NEG(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(PI_CONTROL_REQ_GET_VARIABLE, parent->GetNodeID(), PI_CONTROL_I_NEG,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PIControl - GetI_NEG: " + std::string(e.what()));
    }
}

void PIControl::SetSensorSlope(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("SensorSlope");

    try
    {
        SetVariable(PI_CONTROL_REQ_SET_VARIABLE, parent->GetNodeID(), PI_CONTROL_SENSOR_SLOPE,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PIControl - SetSensorSlope: " + std::string(e.what()));
    }
}

void PIControl::GetSensorSlope(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(PI_CONTROL_REQ_GET_VARIABLE, parent->GetNodeID(), PI_CONTROL_SENSOR_SLOPE,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PIControl - GetSensorSlope: " + std::string(e.what()));
    }
}

void PIControl::SetSensorOffset(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("SensorOffset");

    try
    {
        SetVariable(PI_CONTROL_REQ_SET_VARIABLE, parent->GetNodeID(), PI_CONTROL_SENSOR_OFFSET,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PIControl - SetSensorOffset: " + std::string(e.what()));
    }
}

void PIControl::GetSensorOffset(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(PI_CONTROL_REQ_GET_VARIABLE, parent->GetNodeID(), PI_CONTROL_SENSOR_OFFSET,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PIControl - GetSensorOffset: " + std::string(e.what()));
    }
}

void PIControl::SetOperatingPoint(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("OperatingPoint");

    try
    {
        SetVariable(PI_CONTROL_REQ_SET_VARIABLE, parent->GetNodeID(), PI_CONTROL_OPERATING_POINT,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PIControl - SetOperatingPoint: " + std::string(e.what()));
    }
}

void PIControl::GetOperatingPoint(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(PI_CONTROL_REQ_GET_VARIABLE, parent->GetNodeID(), PI_CONTROL_OPERATING_POINT,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PIControl - GetOperatingPoint: " + std::string(e.what()));
    }
}

void PIControl::SetActuatorChannelID(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("ActuatorChannelID");

    try
    {
        SetVariable(PI_CONTROL_REQ_SET_VARIABLE, parent->GetNodeID(), PI_CONTROL_ACTUATOR_CHANNEL_ID,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PIControl - SetActuatorChannelID: " + std::string(e.what()));
    }
}

void PIControl::GetActuatorChannelID(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(PI_CONTROL_REQ_GET_VARIABLE, parent->GetNodeID(), PI_CONTROL_ACTUATOR_CHANNEL_ID,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PIControl - GetActuatorChannelID: " + std::string(e.what()));
    }
}

void PIControl::SetSensorChannelID(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("SensorChannelID");

    try
    {
        SetVariable(PI_CONTROL_REQ_SET_VARIABLE, parent->GetNodeID(), PI_CONTROL_SENSOR_CHANNEL_ID,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PIControl - SetSensorChannelID: " + std::string(e.what()));
    }
}

void PIControl::GetSensorChannelID(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(PI_CONTROL_REQ_GET_VARIABLE, parent->GetNodeID(), PI_CONTROL_SENSOR_CHANNEL_ID,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PIControl - GetSensorChannelID: " + std::string(e.what()));
    }
}

void PIControl::SetRefreshDivider(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("RefreshDivider");

    try
    {
        SetVariable(PI_CONTROL_REQ_SET_VARIABLE, parent->GetNodeID(), PI_CONTROL_REFRESH_DIVIDER,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PIControl - SetRefreshDivider: " + std::string(e.what()));
    }
}

void PIControl::GetRefreshDivider(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(PI_CONTROL_REQ_GET_VARIABLE, parent->GetNodeID(), PI_CONTROL_REFRESH_DIVIDER,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PIControl - GetRefreshDivider: " + std::string(e.what()));
    }
}

void PIControl::RequestStatus(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, parent->GetNodeID(), PI_CONTROL_REQ_STATUS, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PIControl - RequestStatus: " + std::string(e.what()));
    }
}

void PIControl::RequestResetSettings(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, parent->GetNodeID(), PI_CONTROL_REQ_RESET_SETTINGS, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PIControl - RequestResetSettings: " + std::string(e.what()));
    }
}

void PIControl::RequestCurrentState()
{
    std::vector<double> params;

    GetEnabled(params, false);
    GetTarget(params, false);
    GetP_POS(params, false);
    GetI_POS(params, false);
    GetP_NEG(params, false);
    GetI_NEG(params, false);
    GetSensorSlope(params, false);
    GetSensorOffset(params, false);
    GetOperatingPoint(params, false);
    GetActuatorChannelID(params, false);
    GetSensorChannelID(params, false);
    GetRefreshDivider(params, false);
}