//
// Created by raffael on 31.03.25.
//

#include "can/channels/CANMonitor.h"

#include <vector>


CANMonitor::CANMonitor(uint8_t channelID, std::string channelName, Node *parent)
    : Channel("CANMonitor", channelID, std::move(channelName),sensorScaling, parent, CAN_MONITOR_DATA_N_BYTES), NonNodeChannel(parent) {
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
    nameValueMap.emplace_back(this->GetSensorName(), FDCAN_StatusRegisters::ECR.get(*valuePtr));

    nameValueMap.emplace_back(this->GetStatePrefix() + "TEC", FDCAN_StatusRegisters::ECR_Fields::TEC.get(*valuePtr));
    nameValueMap.emplace_back(this->GetStatePrefix() + "REC", FDCAN_StatusRegisters::ECR_Fields::REC.get(*valuePtr));
    nameValueMap.emplace_back(this->GetStatePrefix() + "RP", FDCAN_StatusRegisters::ECR_Fields::RP.get(*valuePtr));
    nameValueMap.emplace_back(this->GetStatePrefix() + "CEL", FDCAN_StatusRegisters::ECR_Fields::CEL.get(*valuePtr));

    nameValueMap.emplace_back(this->GetStatePrefix() + "LEC", FDCAN_StatusRegisters::PSR_Fields::LEC.get(*valuePtr));
    nameValueMap.emplace_back(this->GetStatePrefix() + "ACT", FDCAN_StatusRegisters::PSR_Fields::ACT.get(*valuePtr));
    nameValueMap.emplace_back(this->GetStatePrefix() + "EP", FDCAN_StatusRegisters::PSR_Fields::EP.get(*valuePtr));
    nameValueMap.emplace_back(this->GetStatePrefix() + "EW", FDCAN_StatusRegisters::PSR_Fields::EW.get(*valuePtr));
    nameValueMap.emplace_back(this->GetStatePrefix() + "DLEC", FDCAN_StatusRegisters::PSR_Fields::DLEC.get(*valuePtr));
    nameValueMap.emplace_back(this->GetStatePrefix() + "RESI", FDCAN_StatusRegisters::PSR_Fields::RESI.get(*valuePtr));
    nameValueMap.emplace_back(this->GetStatePrefix() + "RBRS", FDCAN_StatusRegisters::PSR_Fields::RBRS.get(*valuePtr));
    nameValueMap.emplace_back(this->GetStatePrefix() + "REDL", FDCAN_StatusRegisters::PSR_Fields::REDL.get(*valuePtr));
    nameValueMap.emplace_back(this->GetStatePrefix() + "PXE", FDCAN_StatusRegisters::PSR_Fields::PXE.get(*valuePtr));
    nameValueMap.emplace_back(this->GetStatePrefix() + "TDCV", FDCAN_StatusRegisters::PSR_Fields::TDCV.get(*valuePtr));
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