//
// Created by Markus on 31.03.21.
//

#include "can/channels/IMU.h"
#include "../../../include/StateController.h"

#include <vector>

const std::vector<std::string> IMU::states =
        {
            "Measurement",
            "RefreshDivider",
            "Calibrate",
            "RequestStatus",
            "ResetAllSettings"
        };

const std::map<std::string, std::vector<double>> IMU::scalingMap =
        {
            {"Measurement", {1.0, 0.0}},
            {"RefreshDivider", {1.0, 0.0}},
        };

const std::map<IMU_VARIABLES , std::string> IMU::variableMap =
        {
            {IMU_MEASUREMENT, "Measurement"},
            {IMU_REFRESH_DIVIDER, "RefreshDivider"},
        };

IMU::IMU(uint8_t channelID, std::string channelName, std::vector<double> sensorScaling, Node *parent)
        : Channel("IMU", channelID, std::move(channelName), sensorScaling, parent, IMU_DATA_N_BYTES), NonNodeChannel(parent)
{
    commandMap = {
        {"SetMeasurement", {std::bind(&IMU::SetMeasurement, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetMeasurement", {std::bind(&IMU::GetMeasurement, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"SetRefreshDivider", {std::bind(&IMU::SetRefreshDivider, this, std::placeholders::_1, std::placeholders::_2), {"Value"}}},
        {"GetRefreshDivider", {std::bind(&IMU::GetRefreshDivider, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"RequestCalibrate", {std::bind(&IMU::RequestCalibrate, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"RequestStatus", {std::bind(&IMU::RequestStatus, this, std::placeholders::_1, std::placeholders::_2), {}}},
        {"RequestResetSettings", {std::bind(&IMU::RequestResetSettings, this, std::placeholders::_1, std::placeholders::_2), {}}},
    };
}

//---------------------------------------------------------------------------------------//
//-------------------------------GETTER & SETTER Functions-------------------------------//
//---------------------------------------------------------------------------------------//

std::vector<std::string> IMU::GetStates()
{
    std::vector<std::string> states = IMU::states;
    for (auto &state : states)
    {
        state = GetStatePrefix() + state;
    }
    return states;
}

//-------------------------------------------------------------------------------//
//-------------------------------RECEIVE Functions-------------------------------//
//-------------------------------------------------------------------------------//

void IMU::ProcessCANCommand(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp)
{
    try
    {
        switch (canMsg->bit.cmd_id)
        {
            case IMU_RES_GET_VARIABLE:
            case IMU_RES_SET_VARIABLE:
                GetSetVariableResponse<IMU_VARIABLES>(canMsg, canMsgLength, timestamp, variableMap, scalingMap);
                break;
            case IMU_RES_STATUS:
                StatusResponse(canMsg, canMsgLength, timestamp);
                break;
            case IMU_RES_RESET_SETTINGS:
                ResetSettingsResponse(canMsg, canMsgLength, timestamp);
                break;
            case IMU_RES_CALIBRATE:
                CalibrateResponse(canMsg, canMsgLength, timestamp);
                break;
            case IMU_REQ_RESET_SETTINGS:
            case IMU_REQ_STATUS:
            case IMU_REQ_SET_VARIABLE:
            case IMU_REQ_GET_VARIABLE:
            case IMU_REQ_CALIBRATE:
                throw std::runtime_error("request message type has been received, major fault in protocol");
                break;
            default:
                throw std::runtime_error("IMU specific command with command id not supported: " + std::to_string(canMsg->bit.cmd_id));
        }
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("IMU '" + this->channelName + "' - ProcessCANCommand: " + std::string(e.what()));
    }
}

void IMU::CalibrateResponse(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp)
{
    SetState("Calibrate", 1, timestamp);
}

//----------------------------------------------------------------------------//
//-------------------------------SEND Functions-------------------------------//
//----------------------------------------------------------------------------//

void IMU::SetMeasurement(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("Measurement");

    try
    {
        SetVariable(IMU_REQ_SET_VARIABLE, parent->GetNodeID(), IMU_MEASUREMENT,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("IMU - SetMeasurement: " + std::string(e.what()));
    }
}

void IMU::GetMeasurement(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(IMU_REQ_GET_VARIABLE, parent->GetNodeID(), IMU_MEASUREMENT,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("IMU - GetMeasurement: " + std::string(e.what()));
    }
}

void IMU::SetRefreshDivider(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("RefreshDivider");

    try
    {
        SetVariable(IMU_REQ_SET_VARIABLE, parent->GetNodeID(), IMU_REFRESH_DIVIDER,
                    scalingParams, params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("IMU - SetRefreshDivider: " + std::string(e.what()));
    }
}

void IMU::GetRefreshDivider(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(IMU_REQ_GET_VARIABLE, parent->GetNodeID(), IMU_REFRESH_DIVIDER,
                    params, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("IMU - GetRefreshDivider: " + std::string(e.what()));
    }
}

void IMU::RequestCalibrate(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, parent->GetNodeID(), IMU_REQ_CALIBRATE, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("IMU - RequestCalibrate: " + std::string(e.what()));
    }
}

void IMU::RequestStatus(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, parent->GetNodeID(), IMU_REQ_STATUS, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("IMU - RequestStatus: " + std::string(e.what()));
    }
}

void IMU::RequestResetSettings(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, parent->GetNodeID(), IMU_REQ_RESET_SETTINGS, parent->GetCANBusChannelID(), parent->GetCANDriver(), testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("IMU - RequestResetSettings: " + std::string(e.what()));
    }
}

void IMU::RequestCurrentState()
{
    std::vector<double> params;
    
	GetMeasurement(params, false);
	GetRefreshDivider(params, false);
}