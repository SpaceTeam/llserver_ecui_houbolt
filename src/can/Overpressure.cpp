#include "../../include/can/Overpressure.h"


const std::vector<std::string> Overpressure::states =
{
    "RefreshDivider",
    "Threshold",
    "OperatingMode"
    "RequestStatus",
    "ResetAllSettings"
};

const std::map<std::string, std::vector<double> > Overpressure::scalingMap =
{
    {"Threshold", {1.0, 0.0}},
    {"OperatingMode", {1.0, 0.0}},
    {"RefreshDivider", {1.0, 0.0}},

};


const std::map<OVERPRESSURE_VARIABLES, std::string> Overpressure::variableMap =
{
    {OVERPRESSURE_THRESHOLD, "Threshold"},
    {OVERPRESSURE_OPERATING_MODE, "OperatingMode"},
    {OVERPRESSURE_REFRESH_DIVIDER, "RefreshDivider"}
};

Overpressure::Overpressure(uint8_t channelID, std::string channelName, std::vector<double> sensorScaling, Node *parent)
    : Channel("Overpressure", channelID, std::move(channelName), sensorScaling, parent, OVERPRESSURE_N_BYTES),
      NonNodeChannel(parent) {
    commandMap = {
        {
            "SetThreshold",
            {  [this](std::vector<double> &params, bool testOnly) {SetThreshold(params, testOnly); }, {"Value"}}
        },
        {
            "GetThreshold",
            {  [this](std::vector<double> &params, bool testOnly) {GetThreshold(params, testOnly); }, {}}
        },{
            "SetOperatingMode",
            {  [this](std::vector<double> &params, bool testOnly) {SetOperatingMode(params, testOnly); }, {"Value"}}
        },
        {
            "GetOperatingMode",
            {  [this](std::vector<double> &params, bool testOnly) {GetOperatingMode(params, testOnly); }, {}}
        },{
            "SetRefreshDivider",
            {  [this](std::vector<double> &params, bool testOnly) {SetRefreshDivider(params, testOnly); }, {"Value"}}
        },
        {
            "GetRefreshDivider",
            {  [this](std::vector<double> &params, bool testOnly) {GetRefreshDivider(params, testOnly); }, {}}
        },
        {
            "RequestStatus",
            {
                [this](std::vector<double> &params, bool testOnly) {
                    RequestStatus(params, testOnly);
                },
                {}
            }
        },
        {
            "RequestResetSettings",
            {
                [this](std::vector<double> &params, bool testOnly) {
                    RequestResetSettings(params, testOnly);
                },
                {}
            }
        },
    };
}

//---------------------------------------------------------------------------------------//
//-------------------------------GETTER & SETTER Functions-------------------------------//
//---------------------------------------------------------------------------------------//

std::vector<std::string> Overpressure::GetStates() {
    std::vector<std::string> states = Overpressure::states;
    for (auto &state: states) {
        state = GetStatePrefix() + state;
    }
    return states;
}

//-------------------------------------------------------------------------------//
//-------------------------------RECEIVE Functions-------------------------------//
//-------------------------------------------------------------------------------//

void Overpressure::ProcessCANCommand(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp) {
    try {
        switch (canMsg->bit.cmd_id) {
            case OVERPRESSURE_RES_GET_VARIABLE:
            case OVERPRESSURE_RES_SET_VARIABLE:
                GetSetVariableResponse<
                    OVERPRESSURE_VARIABLES>(canMsg, canMsgLength, timestamp, variableMap, scalingMap);
                break;
            case OVERPRESSURE_RES_STATUS:
                StatusResponse(canMsg, canMsgLength, timestamp);
                break;
            case OVERPRESSURE_RES_RESET_SETTINGS:
                ResetSettingsResponse(canMsg, canMsgLength, timestamp);
                break;
            case OVERPRESSURE_REQ_RESET_SETTINGS:
            case OVERPRESSURE_REQ_STATUS:
            case OVERPRESSURE_REQ_SET_VARIABLE:
            case OVERPRESSURE_REQ_GET_VARIABLE:
                throw std::runtime_error("request message type has been received, major fault in protocol");
                break;
            default:
                throw std::runtime_error(
                    "Overpressure specific command with command id not supported: " +
                    std::to_string(canMsg->bit.cmd_id));
        }
    } catch (std::exception &e) {
        throw std::runtime_error(
            "Overpressure '" + this->channelName + "' - ProcessCANCommand: " + std::string(e.what()));
    }
}

//----------------------------------------------------------------------------//
//-------------------------------SEND Functions-------------------------------//
//----------------------------------------------------------------------------//

void Overpressure::SetRefreshDivider(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("RefreshDivider");

    try
    {
        SetVariable(OVERPRESSURE_REQ_SET_VARIABLE, parent->GetNodeID(), OVERPRESSURE_REFRESH_DIVIDER,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Overpressure - SetRefreshDivider: " + std::string(e.what()));
    }
}

void Overpressure::GetRefreshDivider(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(OVERPRESSURE_REQ_GET_VARIABLE, parent->GetNodeID(), OVERPRESSURE_REFRESH_DIVIDER,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Overpressure - GetRefreshDivider: " + std::string(e.what()));
    }
}

void Overpressure::SetThreshold(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("Threshold");

    try
    {
        SetVariable(OVERPRESSURE_REQ_SET_VARIABLE, parent->GetNodeID(), OVERPRESSURE_THRESHOLD,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Overpressure - SetThreshold: " + std::string(e.what()));
    }
}

void Overpressure::GetThreshold(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(OVERPRESSURE_REQ_GET_VARIABLE, parent->GetNodeID(), OVERPRESSURE_THRESHOLD,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Overpressure - GetThreshold: " + std::string(e.what()));
    }
}

void Overpressure::SetOperatingMode(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("OperatingMode");

    try
    {
        SetVariable(OVERPRESSURE_REQ_SET_VARIABLE, parent->GetNodeID(), OVERPRESSURE_OPERATING_MODE,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Overpressure - SetOperatingMode: " + std::string(e.what()));
    }
}

void Overpressure::GetOperatingMode(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(OVERPRESSURE_REQ_GET_VARIABLE, parent->GetNodeID(), OVERPRESSURE_OPERATING_MODE,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Overpressure - GetOperatingMode: " + std::string(e.what()));
    }
}




void Overpressure::RequestStatus(std::vector<double> &params, bool testOnly) {
    try {
        SendNoPayloadCommand(params, parent->GetNodeID(), OVERPRESSURE_REQ_STATUS, parent->GetCANBusChannelID(),
                             parent->GetCANDriver(), testOnly);
    } catch (std::exception &e) {
        throw std::runtime_error("Overpressure - RequestStatus: " + std::string(e.what()));
    }
}

void Overpressure::RequestResetSettings(std::vector<double> &params, bool testOnly) {
    try {
        SendNoPayloadCommand(params, parent->GetNodeID(), OVERPRESSURE_REQ_RESET_SETTINGS, parent->GetCANBusChannelID(),
                             parent->GetCANDriver(), testOnly);
    } catch (std::exception &e) {
        throw std::runtime_error("Overpressure - RequestResetSettings: " + std::string(e.what()));
    }
}

void Overpressure::RequestCurrentState() {
    std::vector<double> params;

    GetRefreshDivider(params, false);
}
