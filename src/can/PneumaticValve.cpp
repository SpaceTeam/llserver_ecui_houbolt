//
// Created by Markus on 03.09.21.
//

#include "can/PneumaticValve.h"

const std::vector<std::string> PneumaticValve::states =
        {
            "Position",
            "TargetPosition",
            "P",
            "I",
            "D",
            "OnChannelID",
            "OffChannelID",
            "PosChannelID",
            "RefreshDivider",
            "RequestStatus",
            "ResetAllSettings"
        };

const std::map<std::string, std::vector<double>> PneumaticValve::scalingMap =
        {
            {"Position", {1.0, 0.0}},
            {"TargetPosition", {1.0, 0.0}},
            {"P", {1.0, 0.0}},
            {"I", {1.0, 0.0}},
            {"D", {1.0, 0.0}},
            {"OnChannelID", {1.0, 0.0}},
            {"OffChannelID", {1.0, 0.0}},
            {"PosChannelID", {1.0, 0.0}},
            {"MovePosition", {1.0, 0.0}},
            {"MoveInterval", {1.0, 0.0}},
            {"RefreshDivider", {1.0, 0.0}},
        };

const std::map<PNEUMATIC_VALVE_VARIABLES , std::string> PneumaticValve::variableMap =
        {
            {PNEUMATIC_VALVE_POSITION, "Position"},
            {PNEUMATIC_VALVE_TARGET_POSITION, "TargetPosition"},
            {PNEUMATIC_VALVE_P_PARAM, "P"},
            {PNEUMATIC_VALVE_I_PARAM, "I"},
            {PNEUMATIC_VALVE_D_PARAM, "D"},
            {PNEUMATIC_VALVE_ON_CHANNEL_ID, "OnChannelID"},
            {PNEUMATIC_VALVE_OFF_CHANNEL_ID, "OffChannelID"},
            {PNEUMATIC_VALVE_POS_CHANNEL_ID, "PosChannelID"},
            {PNEUMATIC_VALVE_POS_REFRESH_DIVIDER, "RefreshDivider"},
        };

PneumaticValve::PneumaticValve(uint8_t channelID, std::string channelName, std::vector<double> sensorScaling, Node *parent)
        : Channel(channelID, std::move(channelName), sensorScaling, parent, PNEUMATIC_VALVE_DATA_N_BYTES), NonNodeChannel(parent)
{
    commandMap = {
        {"SetPosition", {std::bind(&PneumaticValve::SetPosition, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetPosition", {std::bind(&PneumaticValve::GetPosition, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetTargetPosition", {std::bind(&PneumaticValve::SetTargetPosition, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetTargetPosition", {std::bind(&PneumaticValve::GetTargetPosition, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetP", {std::bind(&PneumaticValve::SetP, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetP", {std::bind(&PneumaticValve::GetP, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetI", {std::bind(&PneumaticValve::SetI, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetI", {std::bind(&PneumaticValve::GetI, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetD", {std::bind(&PneumaticValve::SetD, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetD", {std::bind(&PneumaticValve::GetD, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetOnChannelID", {std::bind(&PneumaticValve::SetOnChannelID, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetOnChannelID", {std::bind(&PneumaticValve::GetOnChannelID, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetOffChannelID", {std::bind(&PneumaticValve::SetOffChannelID, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetOffChannelID", {std::bind(&PneumaticValve::GetOffChannelID, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetPosChannelID", {std::bind(&PneumaticValve::SetPosChannelID, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetPosChannelID", {std::bind(&PneumaticValve::GetPosChannelID, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetRefreshDivider", {std::bind(&PneumaticValve::SetRefreshDivider, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetRefreshDivider", {std::bind(&PneumaticValve::GetRefreshDivider, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"RequestStatus", {std::bind(&PneumaticValve::RequestStatus, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"RequestResetSettings", {std::bind(&PneumaticValve::RequestResetSettings, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"RequestMove", {std::bind(&PneumaticValve::RequestMove, this, std::placeholders::_1, std::placeholders::_2),{"Position","TimeInterval"}}},
    };
}

//---------------------------------------------------------------------------------------//
//-------------------------------GETTER & SETTER Functions-------------------------------//
//---------------------------------------------------------------------------------------//

std::vector<std::string> PneumaticValve::GetStates()
{
    std::vector<std::string> states = PneumaticValve::states;
    for (auto &state : states)
    {
        state = GetStatePrefix() + state;
    }
    return states;
}

//-------------------------------------------------------------------------------//
//-------------------------------RECEIVE Functions-------------------------------//
//-------------------------------------------------------------------------------//

void PneumaticValve::ProcessCANCommand(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp)
{
    try
    {
        switch (canMsg->bit.cmd_id)
        {
            case PNEUMATIC_VALVE_RES_GET_VARIABLE:
            case PNEUMATIC_VALVE_RES_SET_VARIABLE:
                GetSetVariableResponse<PNEUMATIC_VALVE_VARIABLES>(canMsg, canMsgLength, timestamp, variableMap, scalingMap);
                break;
            case PNEUMATIC_VALVE_RES_STATUS:
                StatusResponse(canMsg, canMsgLength, timestamp);
                break;
            case PNEUMATIC_VALVE_RES_RESET_SETTINGS:
                ResetSettingsResponse(canMsg, canMsgLength, timestamp);
                break;
            case PNEUMATIC_VALVE_REQ_RESET_SETTINGS:
            case PNEUMATIC_VALVE_REQ_STATUS:
            case PNEUMATIC_VALVE_REQ_SET_VARIABLE:
            case PNEUMATIC_VALVE_REQ_GET_VARIABLE:
                //TODO: comment out after testing
                //throw std::runtime_error("request message type has been received, major fault in protocol");
                break;
            default:
                throw std::runtime_error("PneumaticValve specific command with command id not supported: " + std::to_string(canMsg->bit.cmd_id));
        }
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PneumaticValve '" + this->channelName + "' - ProcessCANCommand: " + std::string(e.what()));
    }
}

//----------------------------------------------------------------------------//
//-------------------------------SEND Functions-------------------------------//
//----------------------------------------------------------------------------//

void PneumaticValve::SetPosition(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("Position");

    try
    {
        SetVariable(PNEUMATIC_VALVE_REQ_SET_VARIABLE, parent->GetNodeID(), PNEUMATIC_VALVE_POSITION,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PneumaticValve - SetPosition: " + std::string(e.what()));
    }
}

void PneumaticValve::GetPosition(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(PNEUMATIC_VALVE_REQ_GET_VARIABLE, parent->GetNodeID(), PNEUMATIC_VALVE_POSITION,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PneumaticValve - GetPosition: " + std::string(e.what()));
    }
}

void PneumaticValve::SetTargetPosition(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("TargetPosition");

    try
    {
        SetVariable(PNEUMATIC_VALVE_REQ_SET_VARIABLE, parent->GetNodeID(), PNEUMATIC_VALVE_TARGET_POSITION,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PneumaticValve - SetTargetPosition: " + std::string(e.what()));
    }
}

void PneumaticValve::GetTargetPosition(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(PNEUMATIC_VALVE_REQ_GET_VARIABLE, parent->GetNodeID(), PNEUMATIC_VALVE_TARGET_POSITION,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PneumaticValve - GetTargetPosition: " + std::string(e.what()));
    }
}

void PneumaticValve::SetP(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("P");

    try
    {
        SetVariable(PNEUMATIC_VALVE_REQ_SET_VARIABLE, parent->GetNodeID(), PNEUMATIC_VALVE_P_PARAM,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PneumaticValve - SetP: " + std::string(e.what()));
    }
}

void PneumaticValve::GetP(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(PNEUMATIC_VALVE_REQ_GET_VARIABLE, parent->GetNodeID(), PNEUMATIC_VALVE_P_PARAM,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PneumaticValve - GetP: " + std::string(e.what()));
    }
}

void PneumaticValve::SetI(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("I");

    try
    {
        SetVariable(PNEUMATIC_VALVE_REQ_SET_VARIABLE, parent->GetNodeID(), PNEUMATIC_VALVE_I_PARAM,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PneumaticValve - SetI: " + std::string(e.what()));
    }
}

void PneumaticValve::GetI(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(PNEUMATIC_VALVE_REQ_GET_VARIABLE, parent->GetNodeID(), PNEUMATIC_VALVE_I_PARAM,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PneumaticValve - GetI: " + std::string(e.what()));
    }
}

void PneumaticValve::SetD(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("D");

    try
    {
        SetVariable(PNEUMATIC_VALVE_REQ_SET_VARIABLE, parent->GetNodeID(), PNEUMATIC_VALVE_D_PARAM,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PneumaticValve - SetD: " + std::string(e.what()));
    }
}

void PneumaticValve::GetD(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(PNEUMATIC_VALVE_REQ_GET_VARIABLE, parent->GetNodeID(), PNEUMATIC_VALVE_D_PARAM,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PneumaticValve - GetD: " + std::string(e.what()));
    }
}

void PneumaticValve::SetOnChannelID(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("OnChannelID");

    try
    {
        SetVariable(PNEUMATIC_VALVE_REQ_SET_VARIABLE, parent->GetNodeID(), PNEUMATIC_VALVE_ON_CHANNEL_ID,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PneumaticValve - SetOnChannelID: " + std::string(e.what()));
    }
}

void PneumaticValve::GetOnChannelID(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(PNEUMATIC_VALVE_REQ_GET_VARIABLE, parent->GetNodeID(), PNEUMATIC_VALVE_ON_CHANNEL_ID,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PneumaticValve - GetOnChannelID: " + std::string(e.what()));
    }
}

void PneumaticValve::SetOffChannelID(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("OffChannelID");

    try
    {
        SetVariable(PNEUMATIC_VALVE_REQ_SET_VARIABLE, parent->GetNodeID(), PNEUMATIC_VALVE_OFF_CHANNEL_ID,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PneumaticValve - SetOffChannelID: " + std::string(e.what()));
    }
}

void PneumaticValve::GetOffChannelID(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(PNEUMATIC_VALVE_REQ_GET_VARIABLE, parent->GetNodeID(), PNEUMATIC_VALVE_OFF_CHANNEL_ID,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PneumaticValve - GetOffChannelID: " + std::string(e.what()));
    }
}

void PneumaticValve::SetPosChannelID(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("PosChannelID");

    try
    {
        SetVariable(PNEUMATIC_VALVE_REQ_SET_VARIABLE, parent->GetNodeID(), PNEUMATIC_VALVE_POS_CHANNEL_ID,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PneumaticValve - SetPosChannelID: " + std::string(e.what()));
    }
}

void PneumaticValve::GetPosChannelID(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(PNEUMATIC_VALVE_REQ_GET_VARIABLE, parent->GetNodeID(), PNEUMATIC_VALVE_POS_CHANNEL_ID,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PneumaticValve - GetPosChannelID: " + std::string(e.what()));
    }
}

void PneumaticValve::SetRefreshDivider(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("RefreshDivider");

    try
    {
        SetVariable(PNEUMATIC_VALVE_REQ_SET_VARIABLE, parent->GetNodeID(), PNEUMATIC_VALVE_POS_REFRESH_DIVIDER,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PneumaticValve - SetRefreshDivider: " + std::string(e.what()));
    }
}

void PneumaticValve::GetRefreshDivider(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(PNEUMATIC_VALVE_REQ_GET_VARIABLE, parent->GetNodeID(), PNEUMATIC_VALVE_POS_REFRESH_DIVIDER,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PneumaticValve - GetRefreshDivider: " + std::string(e.what()));
    }
}

void PneumaticValve::RequestMove(std::vector<double> &params, bool testOnly)
{
    try
    {
        if (params.size() != 2) //number of required parameters
        {
            throw std::runtime_error("2 parameters expected, but " + std::to_string(params.size()) + " were provided");
        }
        std::vector<double> scalingPosition = scalingMap.at("MovePosition");
        std::vector<double> scalingInterval = scalingMap.at("MoveInterval");

        PneumaticValveMoveMsg_t moveMsg = {0};
        moveMsg.position = Channel::ScaleAndConvertInt32(params[0],scalingPosition[0],scalingPosition[1]);
        moveMsg.interval = Channel::ScaleAndConvertInt32(params[1],scalingInterval[0],scalingInterval[1]);

        SendStandardCommand(parent->GetNodeID(), PNEUMATIC_VALVE_REQ_MOVE, (uint8_t *) &moveMsg, sizeof(moveMsg), parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PneumaticValve - RequestMove: " + std::string(e.what()));
    }
}

void PneumaticValve::RequestStatus(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, parent->GetNodeID(), PNEUMATIC_VALVE_REQ_STATUS, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PneumaticValve - RequestStatus: " + std::string(e.what()));
    }
}

void PneumaticValve::RequestResetSettings(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, parent->GetNodeID(), PNEUMATIC_VALVE_REQ_RESET_SETTINGS, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("PneumaticValve - RequestResetSettings: " + std::string(e.what()));
    }
}