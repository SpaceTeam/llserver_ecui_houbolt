//
// Created by Markus on 31.08.21.
//

#include "can/Servo.h"

const std::vector<std::string> Servo::states =
    {
        "Position",
        "TargetPosition",
        "TargetPressure",
        "MaxSpeed",
        "MaxAccel",
        "MaxTorque",
        "P",
        "I",
        "D",
        "SensorChannelID",
        "Startpoint",
        "Endpoint",
        "PWMEnabled",
        "PositionRaw",
        "RefreshDivider",
        "RequestStatus",
        "ResetAllSettings"};

const std::map<std::string, std::vector<double>> Servo::scalingMap =
    {
        {"Position", {0.00152590219, 0.0}},
        {"TargetPosition", {0.00152590219, 0.0}},
        {"TargetPressure", {1.0, 0.0}},
        {"MaxSpeed", {1.0, 0.0}},
        {"MaxAccel", {1.0, 0.0}},
        {"MaxTorque", {1.0, 0.0}},
        {"P", {1.0, 0.0}},
        {"I", {1.0, 0.0}},
        {"D", {1.0, 0.0}},
        {"SensorChannelID", {1.0, 0.0}},
        {"Startpoint", {1.0, 0.0}},
        {"Endpoint", {1.0, 0.0}},
        {"PWMEnabled", {1.0, 0.0}},
        {"PositionRaw", {1.0, 0.0}},
        {"MovePosition", {1.0, 0.0}},
        {"MoveInterval", {1.0, 0.0}},
        {"RefreshDivider", {1.0, 0.0}},
};

const std::map<SERVO_VARIABLES, std::string> Servo::variableMap =
    {
        {SERVO_POSITION, "Position"},
        {SERVO_TARGET_POSITION, "TargetPosition"},
        {SERVO_TARGET_PRESSURE, "TargetPressure"},
        {SERVO_MAX_SPEED, "MaxSpeed"},
        {SERVO_MAX_ACCEL, "MaxAccel"},
        {SERVO_MAX_TORQUE, "MaxTorque"},
        {SERVO_P_PARAM, "P"},
        {SERVO_I_PARAM, "I"},
        {SERVO_D_PARAM, "D"},
        {SERVO_SENSOR_CHANNEL_ID, "SensorChannelID"},
        {SERVO_POSITION_STARTPOINT, "Startpoint"},
        {SERVO_POSITION_ENDPOINT, "Endpoint"},
        {SERVO_PWM_ENABLED, "PWMEnabled"},
        {SERVO_POSITION_RAW, "PositionRaw"},
        {SERVO_SENSOR_REFRESH_DIVIDER, "RefreshDivider"},
};

Servo::Servo(uint8_t channelID, std::string channelName, std::vector<double> sensorScaling, Node *parent)
    : Channel("Servo", channelID, std::move(channelName), sensorScaling, parent, SERVO_DATA_N_BYTES), NonNodeChannel(parent)
{
    commandMap = {
        {"SetPosition", {std::bind(&Servo::SetPosition, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetPosition", {std::bind(&Servo::GetPosition, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetTargetPosition", {std::bind(&Servo::SetTargetPosition, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetTargetPosition", {std::bind(&Servo::GetTargetPosition, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetTargetPressure", {std::bind(&Servo::SetTargetPressure, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetTargetPressure", {std::bind(&Servo::GetTargetPressure, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetMaxSpeed", {std::bind(&Servo::SetMaxSpeed, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetMaxSpeed", {std::bind(&Servo::GetMaxSpeed, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetMaxAccel", {std::bind(&Servo::SetMaxAccel, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetMaxAccel", {std::bind(&Servo::GetMaxAccel, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetMaxTorque", {std::bind(&Servo::SetMaxTorque, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetMaxTorque", {std::bind(&Servo::GetMaxTorque, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetP", {std::bind(&Servo::SetP, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetP", {std::bind(&Servo::GetP, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetI", {std::bind(&Servo::SetI, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetI", {std::bind(&Servo::GetI, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetD", {std::bind(&Servo::SetD, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetD", {std::bind(&Servo::GetD, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetSensorChannelID", {std::bind(&Servo::SetSensorChannelID, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetSensorChannelID", {std::bind(&Servo::GetSensorChannelID, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetStartpoint", {std::bind(&Servo::SetStartpoint, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetStartpoint", {std::bind(&Servo::GetStartpoint, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetEndpoint", {std::bind(&Servo::SetEndpoint, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetEndpoint", {std::bind(&Servo::GetEndpoint, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetPWMEnabled", {std::bind(&Servo::SetPWMEnabled, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetPWMEnabled", {std::bind(&Servo::GetPWMEnabled, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetPositionRaw", {std::bind(&Servo::SetPositionRaw, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetPositionRaw", {std::bind(&Servo::GetPositionRaw, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetRefreshDivider", {std::bind(&Servo::SetRefreshDivider, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetRefreshDivider", {std::bind(&Servo::GetRefreshDivider, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"RequestStatus", {std::bind(&Servo::RequestStatus, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"RequestResetSettings", {std::bind(&Servo::RequestResetSettings, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"RequestMove", {std::bind(&Servo::RequestMove, this, std::placeholders::_1, std::placeholders::_2), {"Position", "TimeInterval"}}},
    };
}

//---------------------------------------------------------------------------------------//
//-------------------------------GETTER & SETTER Functions-------------------------------//
//---------------------------------------------------------------------------------------//

std::vector<std::string> Servo::GetStates()
{
    std::vector<std::string> states = Servo::states;
    for (auto &state : states)
    {
        state = GetStatePrefix() + state;
    }
    return states;
}

//-------------------------------------------------------------------------------//
//-------------------------------RECEIVE Functions-------------------------------//
//-------------------------------------------------------------------------------//

void Servo::ProcessCANCommand(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp)
{
    try
    {
        switch (canMsg->bit.cmd_id)
        {
        case SERVO_RES_GET_VARIABLE:
        case SERVO_RES_SET_VARIABLE:
            GetSetVariableResponse<SERVO_VARIABLES>(canMsg, canMsgLength, timestamp, variableMap, scalingMap);
            break;
        case SERVO_RES_STATUS:
            StatusResponse(canMsg, canMsgLength, timestamp);
            break;
        case SERVO_RES_RESET_SETTINGS:
            ResetSettingsResponse(canMsg, canMsgLength, timestamp);
            break;
        case SERVO_REQ_RESET_SETTINGS:
        case SERVO_REQ_STATUS:
        case SERVO_REQ_SET_VARIABLE:
        case SERVO_REQ_GET_VARIABLE:
            // TODO: comment out after testing
            // throw std::runtime_error("request message type has been received, major fault in protocol");
            break;
        default:
            throw std::runtime_error("Servo specific command with command id not supported: " + std::to_string(canMsg->bit.cmd_id));
        }
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo '" + this->channelName + "' - ProcessCANCommand: " + std::string(e.what()));
    }
}

//----------------------------------------------------------------------------//
//-------------------------------SEND Functions-------------------------------//
//----------------------------------------------------------------------------//

void Servo::SetPosition(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("Position");

    try
    {
        SetVariable(SERVO_REQ_SET_VARIABLE, parent->GetNodeID(), SERVO_POSITION,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - SetPosition: " + std::string(e.what()));
    }
}

void Servo::GetPosition(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(SERVO_REQ_GET_VARIABLE, parent->GetNodeID(), SERVO_POSITION,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - GetPosition: " + std::string(e.what()));
    }
}

void Servo::SetPositionRaw(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("PositionRaw");

    try
    {
        SetVariable(SERVO_REQ_SET_VARIABLE, parent->GetNodeID(), SERVO_POSITION_RAW,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - SetPositionRaw: " + std::string(e.what()));
    }
}

void Servo::GetPositionRaw(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(SERVO_REQ_GET_VARIABLE, parent->GetNodeID(), SERVO_POSITION_RAW,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - GetPositionRaw: " + std::string(e.what()));
    }
}

void Servo::SetTargetPosition(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("TargetPosition");

    try
    {
        SetVariable(SERVO_REQ_SET_VARIABLE, parent->GetNodeID(), SERVO_TARGET_POSITION,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - SetTargetPosition: " + std::string(e.what()));
    }
}

void Servo::GetTargetPosition(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(SERVO_REQ_GET_VARIABLE, parent->GetNodeID(), SERVO_TARGET_POSITION,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - GetTargetPosition: " + std::string(e.what()));
    }
}

void Servo::SetTargetPressure(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("TargetPressure");

    try
    {
        SetVariable(SERVO_REQ_SET_VARIABLE, parent->GetNodeID(), SERVO_TARGET_PRESSURE,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - SetTargetPressure: " + std::string(e.what()));
    }
}

void Servo::GetTargetPressure(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(SERVO_REQ_GET_VARIABLE, parent->GetNodeID(), SERVO_TARGET_PRESSURE,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - GetTargetPressure: " + std::string(e.what()));
    }
}

void Servo::SetMaxSpeed(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("MaxSpeed");

    try
    {
        SetVariable(SERVO_REQ_SET_VARIABLE, parent->GetNodeID(), SERVO_MAX_SPEED,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - SetMaxSpeed: " + std::string(e.what()));
    }
}

void Servo::GetMaxSpeed(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(SERVO_REQ_GET_VARIABLE, parent->GetNodeID(), SERVO_MAX_SPEED,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - GetMaxSpeed: " + std::string(e.what()));
    }
}

void Servo::SetMaxAccel(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("MaxAccel");

    try
    {
        SetVariable(SERVO_REQ_SET_VARIABLE, parent->GetNodeID(), SERVO_MAX_ACCEL,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - SetMaxAccel: " + std::string(e.what()));
    }
}

void Servo::GetMaxAccel(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(SERVO_REQ_GET_VARIABLE, parent->GetNodeID(), SERVO_MAX_ACCEL,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - GetMaxAccel: " + std::string(e.what()));
    }
}

void Servo::SetMaxTorque(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("MaxTorque");

    try
    {
        SetVariable(SERVO_REQ_SET_VARIABLE, parent->GetNodeID(), SERVO_MAX_TORQUE,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - SetMaxTorque: " + std::string(e.what()));
    }
}

void Servo::GetMaxTorque(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(SERVO_REQ_GET_VARIABLE, parent->GetNodeID(), SERVO_MAX_TORQUE,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - GetMaxTorque: " + std::string(e.what()));
    }
}

void Servo::SetP(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("P");

    try
    {
        SetVariable(SERVO_REQ_SET_VARIABLE, parent->GetNodeID(), SERVO_P_PARAM,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - SetP: " + std::string(e.what()));
    }
}

void Servo::GetP(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(SERVO_REQ_GET_VARIABLE, parent->GetNodeID(), SERVO_P_PARAM,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - GetP: " + std::string(e.what()));
    }
}

void Servo::SetI(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("I");

    try
    {
        SetVariable(SERVO_REQ_SET_VARIABLE, parent->GetNodeID(), SERVO_I_PARAM,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - SetI: " + std::string(e.what()));
    }
}

void Servo::GetI(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(SERVO_REQ_GET_VARIABLE, parent->GetNodeID(), SERVO_I_PARAM,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - GetI: " + std::string(e.what()));
    }
}

void Servo::SetD(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("D");

    try
    {
        SetVariable(SERVO_REQ_SET_VARIABLE, parent->GetNodeID(), SERVO_D_PARAM,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - SetD: " + std::string(e.what()));
    }
}

void Servo::GetD(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(SERVO_REQ_GET_VARIABLE, parent->GetNodeID(), SERVO_D_PARAM,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - GetD: " + std::string(e.what()));
    }
}

void Servo::SetSensorChannelID(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("SensorChannelID");

    try
    {
        SetVariable(SERVO_REQ_SET_VARIABLE, parent->GetNodeID(), SERVO_SENSOR_CHANNEL_ID,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - SetSensorChannelID: " + std::string(e.what()));
    }
}

void Servo::GetSensorChannelID(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(SERVO_REQ_GET_VARIABLE, parent->GetNodeID(), SERVO_SENSOR_CHANNEL_ID,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - GetSensorChannelID: " + std::string(e.what()));
    }
}

void Servo::SetStartpoint(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("Startpoint");

    try
    {
        SetVariable(SERVO_REQ_SET_VARIABLE, parent->GetNodeID(), SERVO_POSITION_STARTPOINT,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - SetStartpoint: " + std::string(e.what()));
    }
}

void Servo::GetStartpoint(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(SERVO_REQ_GET_VARIABLE, parent->GetNodeID(), SERVO_POSITION_STARTPOINT,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - GetStartpoint: " + std::string(e.what()));
    }
}

void Servo::SetEndpoint(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("Endpoint");

    try
    {
        SetVariable(SERVO_REQ_SET_VARIABLE, parent->GetNodeID(), SERVO_POSITION_ENDPOINT,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - SetEndpoint: " + std::string(e.what()));
    }
}

void Servo::GetEndpoint(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(SERVO_REQ_GET_VARIABLE, parent->GetNodeID(), SERVO_POSITION_ENDPOINT,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - GetEndpoint: " + std::string(e.what()));
    }
}

void Servo::SetPWMEnabled(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("PWMEnabled");

    try
    {
        SetVariable(SERVO_REQ_SET_VARIABLE, parent->GetNodeID(), SERVO_PWM_ENABLED,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - SetPWMEnabled: " + std::string(e.what()));
    }
}

void Servo::GetPWMEnabled(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(SERVO_REQ_GET_VARIABLE, parent->GetNodeID(), SERVO_PWM_ENABLED,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - GetPWMEnabled: " + std::string(e.what()));
    }
}

void Servo::SetRefreshDivider(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("RefreshDivider");

    try
    {
        SetVariable(SERVO_REQ_SET_VARIABLE, parent->GetNodeID(), SERVO_SENSOR_REFRESH_DIVIDER,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - SetRefreshDivider: " + std::string(e.what()));
    }
}

void Servo::GetRefreshDivider(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(SERVO_REQ_GET_VARIABLE, parent->GetNodeID(), SERVO_SENSOR_REFRESH_DIVIDER,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - GetRefreshDivider: " + std::string(e.what()));
    }
}

void Servo::RequestMove(std::vector<double> &params, bool testOnly)
{
    try
    {
        if (params.size() != 2) // number of required parameters
        {
            throw std::runtime_error("2 parameters expected, but " + std::to_string(params.size()) + " were provided");
        }
        std::vector<double> scalingPosition = scalingMap.at("MovePosition");
        std::vector<double> scalingInterval = scalingMap.at("MoveInterval");

        ServoMoveMsg_t moveMsg = {0};
        moveMsg.position = Channel::ScaleAndConvertInt32(params[0], scalingPosition[0], scalingPosition[1]);
        moveMsg.interval = Channel::ScaleAndConvertInt32(params[1], scalingInterval[0], scalingInterval[1]);

        SendStandardCommand(parent->GetNodeID(), SERVO_REQ_MOVE, (uint8_t *)&moveMsg, sizeof(moveMsg), parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - RequestMove: " + std::string(e.what()));
    }
}

void Servo::RequestStatus(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, parent->GetNodeID(), SERVO_REQ_STATUS, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - RequestStatus: " + std::string(e.what()));
    }
}

void Servo::RequestResetSettings(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, parent->GetNodeID(), SERVO_REQ_RESET_SETTINGS, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Servo - RequestResetSettings: " + std::string(e.what()));
    }
}

void Servo::RequestCurrentState()
{
    std::vector<double> params;

    GetPosition(params, false);
    GetPositionRaw(params, false);
    GetTargetPosition(params, false);
    GetTargetPressure(params, false);
    GetMaxSpeed(params, false);
    GetMaxAccel(params, false);
    GetMaxTorque(params, false);
    GetP(params, false);
    GetI(params, false);
    GetD(params, false);
    GetSensorChannelID(params, false);
    GetStartpoint(params, false);
    GetEndpoint(params, false);
    GetPWMEnabled(params, false);
    GetRefreshDivider(params, false);
}