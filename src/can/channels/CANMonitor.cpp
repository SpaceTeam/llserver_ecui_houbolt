//
// Created by raffael on 31.03.25.
//

#include "can/channels/CANMonitor.h"

#include <vector>

#include "channels/can_monitor_channel_def.h"

CANMonitor::CANMonitor(uint8_t channelID, std::string channelName, Node *parent)
    : Channel("CANMonitor", channelID, std::move(channelName), parent, CAN_MONITOR_CONTROL_DATA_N_BYTES), NonNodeChannel(parent) {
    commandMap = {
        {"SetRefreshDivider", {std::bind(&CANMonitor::SetRefreshDivider, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetRefreshDivider", {std::bind(&CANMonitor::GetRefreshDivider, this, std::placeholders::_1, std::placeholders::_2), {}}},
    };
}


const std::vector<std::string> CANMonitor::states =
        {
            "RefreshDivider",
        };

const std::map<std::string, std::vector<double>> CANMonitor::scalingMap =
        {
            {"RefreshDivider", {1.0, 0.0}},
        };

const std::map<CAN_MONITOR_VARIABLES , std::string> CANMonitor::variableMap =
        {
            {CAN_MONITOR_REFRESH_DIVIDER, "RefreshDivider"},
        };

//---------------------------------------------------------------------------------------//
//-------------------------------GETTER & SETTER Functions-------------------------------//
//---------------------------------------------------------------------------------------//

std::vector<std::string> CANMonitor::GetStates()
{
    std::vector<std::string> states = CANMonitor::states;
    for (auto &state : states)
    {
        state = GetStatePrefix() + state;
    }
    return states;
}

void CANMonitor::GetSensorValue(uint8_t *valuePtr, uint8_t &valueLength, std::vector<std::pair<std::string, double>> &nameValueMap) {
    valueLength = this->typeSize;
    auto registers = reinterpret_cast<FDCAN_StatusRegisters_t*>(valuePtr);
    nameValueMap.emplace_back(std::make_pair(this->GetSensorName(), registers->raw.ecr));

    nameValueMap.emplace_back(std::make_pair(this->GetStatePrefix()+ "TEC", registers->bits.TEC));
    nameValueMap.emplace_back(std::make_pair(this->GetStatePrefix()+ "REC", registers->bits.REC));
    nameValueMap.emplace_back(std::make_pair(this->GetStatePrefix()+ "RP", registers->bits.RP));
    nameValueMap.emplace_back(std::make_pair(this->GetStatePrefix()+ "CEL", registers->bits.CEL));
    nameValueMap.emplace_back(std::make_pair(this->GetStatePrefix()+ "LEC", registers->bits.LEC));
    nameValueMap.emplace_back(std::make_pair(this->GetStatePrefix()+ "ACT", registers->bits.ACT));
    nameValueMap.emplace_back(std::make_pair(this->GetStatePrefix()+ "EP", registers->bits.EP));
    nameValueMap.emplace_back(std::make_pair(this->GetStatePrefix()+ "EW", registers->bits.EW));
    nameValueMap.emplace_back(std::make_pair(this->GetStatePrefix()+ "DLEC", registers->bits.DLEC));
    nameValueMap.emplace_back(std::make_pair(this->GetStatePrefix()+ "RESI", registers->bits.RESI));
    nameValueMap.emplace_back(std::make_pair(this->GetStatePrefix()+ "RBRS", registers->bits.RBRS));
    nameValueMap.emplace_back(std::make_pair(this->GetStatePrefix()+ "REDL", registers->bits.REDL));
    nameValueMap.emplace_back(std::make_pair(this->GetStatePrefix()+ "PXE", registers->bits.PXE));
    nameValueMap.emplace_back(std::make_pair(this->GetStatePrefix()+ "PXE", registers->bits.PXE));
    nameValueMap.emplace_back(std::make_pair(this->GetStatePrefix()+ "TDCV", registers->bits.TDCV));
}


//-------------------------------------------------------------------------------//
//-------------------------------RECEIVE Functions-------------------------------//
//-------------------------------------------------------------------------------//

void CANMonitor::ProcessCANCommand(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp)
{
    try
    {
        switch (canMsg->bit.cmd_id)
        {
            case CAN_MONITOR_RES_GET_VARIABLE:
            case CAN_MONITOR_RES_SET_VARIABLE:
                GetSetVariableResponse<CAN_MONITOR_VARIABLES>(canMsg, canMsgLength, timestamp, variableMap, scalingMap);
                break;
            case CAN_MONITOR_RES_STATUS:
                StatusResponse(canMsg, canMsgLength, timestamp);
                break;
            case CAN_MONITOR_REQ_STATUS:
            case CAN_MONITOR_REQ_SET_VARIABLE:
            case CAN_MONITOR_REQ_GET_VARIABLE:
                throw std::runtime_error("request message type has been received, major fault in protocol");
                break;
            default:
                throw std::runtime_error("CANMonitor specific command with command id not supported: " + std::to_string(canMsg->bit.cmd_id));
        }
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("CANMonitor '" + this->channelName + "' - ProcessCANCommand: " + std::string(e.what()));
    }
}

//----------------------------------------------------------------------------//
//-------------------------------SEND Functions-------------------------------//
//----------------------------------------------------------------------------//

void CANMonitor::SetRefreshDivider(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("RefreshDivider");

    try
    {
        SetVariable(CAN_MONITOR_REQ_SET_VARIABLE, parent->GetNodeID(), CAN_MONITOR_REFRESH_DIVIDER,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("CANMonitor - SetRefreshDivider: " + std::string(e.what()));
    }
}

void CANMonitor::GetRefreshDivider(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(CAN_MONITOR_REQ_GET_VARIABLE, parent->GetNodeID(), CAN_MONITOR_REFRESH_DIVIDER,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("CANMonitor - GetRefreshDivider: " + std::string(e.what()));
    }
}

void CANMonitor::RequestCurrentState()
{
    std::vector<double> params;

	GetRefreshDivider(params, false);
}