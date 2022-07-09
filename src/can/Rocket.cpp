//
// Created by Markus on 31.08.21.
//

#include "can/Rocket.h"

const std::vector<std::string> Rocket::states =
        {
            "State",
            "StateStatus",
            "MinimumChamberPressure",
            "MinimumFuelPressure",
            "MinimumOxPressure",
            "HolddownTimeout",
            "InternalControl",
            "Abort",
            "EndOfFlight",
            "AutoCheck",
            "StateRefreshDivider",
            "RequestStatus",
            "ResetAllSettings"
        };

const std::map<std::string, std::vector<double>> Rocket::scalingMap =
        {
            {"MinimumChamberPressure", {0.0037, 0.0}},
            {"MinimumFuelPressure", {0.00367, 0.0}},
            {"MinimumOxPressure", {0.003735, 0.0}},
            {"HolddownTimeout", {1.0, 0.0}},
            {"StateRefreshDivider", {1.0, 0.0}},
        };

const std::map<ROCKET_VARIABLES , std::string> Rocket::variableMap =
        {
            {ROCKET_MINIMUM_CHAMBER_PRESSURE, "MinimumChamberPressure"},
            {ROCKET_MINIMUM_FUEL_PRESSURE, "MinimumFuelPressure"},
            {ROCKET_MINIMUM_OX_PRESSURE, "MinimumOxPressure"},
            {ROCKET_HOLDDOWN_TIMEOUT, "HolddownTimeout"},
            {ROCKET_STATE_REFRESH_DIVIDER, "StateRefreshDivider"},
        };

Rocket::Rocket(uint8_t channelID, std::string channelName, std::vector<double> sensorScaling, Node *parent)
        : Channel("Rocket", channelID, std::move(channelName), sensorScaling, parent, ROCKET_DATA_N_BYTES), NonNodeChannel(parent)
{
    commandMap = {
        {"SetMinimumChamberPressure", {std::bind(&Rocket::SetMinimumChamberPressure, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetMinimumChamberPressure", {std::bind(&Rocket::GetMinimumChamberPressure, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetMinimumFuelPressure", {std::bind(&Rocket::SetMinimumFuelPressure, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetMinimumFuelPressure", {std::bind(&Rocket::GetMinimumFuelPressure, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetMinimumOxPressure", {std::bind(&Rocket::SetMinimumOxPressure, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetMinimumOxPressure", {std::bind(&Rocket::GetMinimumOxPressure, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetHolddownTimeout", {std::bind(&Rocket::SetHolddownTimeout, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetHolddownTimeout", {std::bind(&Rocket::GetHolddownTimeout, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetStateRefreshDivider", {std::bind(&Rocket::SetStateRefreshDivider, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetStateRefreshDivider", {std::bind(&Rocket::GetStateRefreshDivider, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetRocketState", {std::bind(&Rocket::SetRocketState, this, std::placeholders::_1, std::placeholders::_2), {"State"}}},
        {"GetRocketState", {std::bind(&Rocket::GetRocketState, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"ActivateInternalControl", {std::bind(&Rocket::RequestInternalControl, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"Abort", {std::bind(&Rocket::RequestAbort, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"EndOfFlight", {std::bind(&Rocket::RequestEndOfFlight, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"AutoCheck", {std::bind(&Rocket::RequestAutoCheck, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"RequestStatus", {std::bind(&Rocket::RequestStatus, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"RequestResetSettings", {std::bind(&Rocket::RequestResetSettings, this, std::placeholders::_1, std::placeholders::_2), {}}},
    };
}

//---------------------------------------------------------------------------------------//
//-------------------------------GETTER & SETTER Functions-------------------------------//
//---------------------------------------------------------------------------------------//

std::vector<std::string> Rocket::GetStates()
{
    std::vector<std::string> states = Rocket::states;
    for (auto &state : states)
    {
        state = GetStatePrefix() + state;
    }
    return states;
}

//-------------------------------------------------------------------------------//
//-------------------------------RECEIVE Functions-------------------------------//
//-------------------------------------------------------------------------------//

void Rocket::ProcessCANCommand(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp)
{
    try
    {
        switch (canMsg->bit.cmd_id)
        {
            case ROCKET_RES_GET_VARIABLE:
            case ROCKET_RES_SET_VARIABLE:
                GetSetVariableResponse<ROCKET_VARIABLES>(canMsg, canMsgLength, timestamp, variableMap, scalingMap);
                break;
            case ROCKET_RES_STATUS:
                StatusResponse(canMsg, canMsgLength, timestamp);
                break;
            case ROCKET_RES_RESET_SETTINGS:
                ResetSettingsResponse(canMsg, canMsgLength, timestamp);
                break;
            case ROCKET_RES_GET_ROCKET_STATE:
            case ROCKET_RES_SET_ROCKET_STATE:
                RocketStateResponse(canMsg, canMsgLength, timestamp);
                break;
            case ROCKET_RES_INTERNAL_CONTROL:
                InternalControlResponse(canMsg, canMsgLength, timestamp);
                break;
            case ROCKET_RES_ABORT:
                AbortResponse(canMsg, canMsgLength, timestamp);
                break;
            case ROCKET_RES_END_OF_FLIGHT:
                EndOfFlightResponse(canMsg, canMsgLength, timestamp);
                break;
            case ROCKET_RES_AUTO_CHECK:
                AutoCheckResponse(canMsg, canMsgLength, timestamp);
                break;
            case ROCKET_REQ_RESET_SETTINGS:
            case ROCKET_REQ_STATUS:
            case ROCKET_REQ_SET_VARIABLE:
            case ROCKET_REQ_GET_VARIABLE:
            case ROCKET_REQ_GET_ROCKET_STATE:
            case ROCKET_REQ_SET_ROCKET_STATE:
            case ROCKET_REQ_INTERNAL_CONTROL:
            case ROCKET_REQ_ABORT:
            case ROCKET_REQ_END_OF_FLIGHT:
                //TODO: comment out after testing
                //throw std::runtime_error("request message type has been received, major fault in protocol");
                break;
            default:
                throw std::runtime_error("Rocket specific command with command id not supported: " + std::to_string(canMsg->bit.cmd_id));
        }
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Rocket '" + this->channelName + "' - ProcessCANCommand: " + std::string(e.what()));
    }
}

void Rocket::RocketStateResponse(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp)
{
    RocketStateResMsg_t *resMsg = (RocketStateResMsg_t *) canMsg->bit.data.uint8;
    SetState("State", (double)resMsg->state, timestamp);
    SetState("StateStatus", (double)resMsg->status, timestamp);
}

void Rocket::InternalControlResponse(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp)
{
    SetState("InternalControl", 1, timestamp);
}

void Rocket::AbortResponse(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp)
{
    SetState("Abort", 1, timestamp);
}

void Rocket::EndOfFlightResponse(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp)
{
    SetState("EndOfFlight", 1, timestamp);
}

void Rocket::AutoCheckResponse(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp)
{
    SetState("AutoCheck", 1, timestamp);
}

//----------------------------------------------------------------------------//
//-------------------------------SEND Functions-------------------------------//
//----------------------------------------------------------------------------//

void Rocket::SetMinimumChamberPressure(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("MinimumChamberPressure");

    try
    {
        SetVariable(ROCKET_REQ_SET_VARIABLE, parent->GetNodeID(), ROCKET_MINIMUM_CHAMBER_PRESSURE,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Rocket - SetMinimumChamberPressure: " + std::string(e.what()));
    }
}

void Rocket::GetMinimumChamberPressure(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(ROCKET_REQ_GET_VARIABLE, parent->GetNodeID(), ROCKET_MINIMUM_CHAMBER_PRESSURE,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Rocket - GetMinimumChamberPressure: " + std::string(e.what()));
    }
}

void Rocket::SetMinimumFuelPressure(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("MinimumFuelPressure");

    try
    {
        SetVariable(ROCKET_REQ_SET_VARIABLE, parent->GetNodeID(), ROCKET_MINIMUM_FUEL_PRESSURE,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Rocket - SetMinimumFuelPressure: " + std::string(e.what()));
    }
}

void Rocket::GetMinimumFuelPressure(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(ROCKET_REQ_GET_VARIABLE, parent->GetNodeID(), ROCKET_MINIMUM_FUEL_PRESSURE,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Rocket - GetMinimumFuelPressure: " + std::string(e.what()));
    }
}


void Rocket::SetMinimumOxPressure(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("MinimumOxPressure");

    try
    {
        SetVariable(ROCKET_REQ_SET_VARIABLE, parent->GetNodeID(), ROCKET_MINIMUM_OX_PRESSURE,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Rocket - SetMinimumOxPressure: " + std::string(e.what()));
    }
}

void Rocket::GetMinimumOxPressure(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(ROCKET_REQ_GET_VARIABLE, parent->GetNodeID(), ROCKET_MINIMUM_OX_PRESSURE,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Rocket - GetMinimumOxPressure: " + std::string(e.what()));
    }
}

void Rocket::SetHolddownTimeout(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("HolddownTimeout");

    try
    {
        SetVariable(ROCKET_REQ_SET_VARIABLE, parent->GetNodeID(), ROCKET_HOLDDOWN_TIMEOUT,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Rocket - SetHolddownTimeout: " + std::string(e.what()));
    }
}

void Rocket::GetHolddownTimeout(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(ROCKET_REQ_GET_VARIABLE, parent->GetNodeID(), ROCKET_HOLDDOWN_TIMEOUT,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Rocket - GetHolddownTimeout: " + std::string(e.what()));
    }
}

void Rocket::SetRocketState(std::vector<double> &params, bool testOnly)
{
    try
    {
        if (params.size() != 1) //number of required parameters
        {
            throw std::runtime_error("1 parameter expected, but " + std::to_string(params.size()) + " were provided");
        }
        RocketStateReqMsg_t reqMsg = {0};
        reqMsg.state = Channel::ScaleAndConvertInt32(params[0],1.0,0.0);

        SendStandardCommand(parent->GetNodeID(), ROCKET_REQ_SET_ROCKET_STATE, (uint8_t *) &reqMsg, sizeof(reqMsg), parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Rocket - SetRocketState: " + std::string(e.what()));
    }
}

void Rocket::GetRocketState(std::vector<double> &params, bool testOnly)
{
    try
    {
        if (params.size() != 0) //number of required parameters
        {
            throw std::runtime_error("0 parameters expected, but " + std::to_string(params.size()) + " were provided");
        }
        
        SendNoPayloadCommand(params, parent->GetNodeID(), ROCKET_REQ_GET_ROCKET_STATE, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Rocket - GetRocketState: " + std::string(e.what()));
    }
}

void Rocket::RequestInternalControl(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, parent->GetNodeID(), ROCKET_REQ_INTERNAL_CONTROL, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Rocket - RequestInternalControl: " + std::string(e.what()));
    }
}

void Rocket::RequestAbort(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, parent->GetNodeID(), ROCKET_REQ_ABORT, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Rocket - RequestAbort: " + std::string(e.what()));
    }
}

void Rocket::RequestEndOfFlight(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, parent->GetNodeID(), ROCKET_REQ_END_OF_FLIGHT, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Rocket - RequestEndOfFlight: " + std::string(e.what()));
    }
}

void Rocket::RequestAutoCheck(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, parent->GetNodeID(), ROCKET_REQ_AUTO_CHECK, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Rocket - RequestAutoCheck: " + std::string(e.what()));
    }
}

void Rocket::SetStateRefreshDivider(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("RefreshDivider");

    try
    {
        SetVariable(ROCKET_REQ_SET_VARIABLE, parent->GetNodeID(), ROCKET_STATE_REFRESH_DIVIDER,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Rocket - SetRefreshDivider: " + std::string(e.what()));
    }
}

void Rocket::GetStateRefreshDivider(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(ROCKET_REQ_GET_VARIABLE, parent->GetNodeID(), ROCKET_STATE_REFRESH_DIVIDER,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Rocket - GetRefreshDivider: " + std::string(e.what()));
    }
}

void Rocket::RequestStatus(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, parent->GetNodeID(), ROCKET_REQ_STATUS, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Rocket - RequestStatus: " + std::string(e.what()));
    }
}

void Rocket::RequestResetSettings(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, parent->GetNodeID(), ROCKET_REQ_RESET_SETTINGS, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Rocket - RequestResetSettings: " + std::string(e.what()));
    }
}

void Rocket::RequestCurrentState()
{
    std::vector<double> params;

	GetMinimumChamberPressure(params, false);
	GetMinimumFuelPressure(params, false);
	GetMinimumOxPressure(params, false);
	GetHolddownTimeout(params, false);
	GetStateRefreshDivider(params, false);
    GetRocketState(params, false);
}