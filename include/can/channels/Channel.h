#ifndef LLSERVER_ECUI_HOUBOLT_CHANNEL_H
#define LLSERVER_ECUI_HOUBOLT_CHANNEL_H

#include "common.h"

#include <stdint.h>
#include <string>
#include <cstdarg>
#include <map>
#include <utility>
#include <vector>
#include <functional>
#include <string>
#include <can_houbolt/cmds.h>
#include <limits.h>
#include <atomic>

#include "StateController.h"
#include "can_houbolt/cmds.h"
#include "CANDriver.h"
#include "EventManager.h"

class Channel
{
private:
    //TODO: MP check if this is the only and correct way to implement static const with inheritation
    static const std::vector<std::string> states;
    static const std::map<std::string, std::vector<double>> sensorScalingMap;

protected:

    const std::string channelTypeName = "undefined";

    uint8_t channelID;
    const std::string channelName;
    std::mutex scalingMtx;
    std::vector<double> sensorScaling;
    const uint8_t typeSize; //in bytes

    std::map<std::string, command_t> commandMap;

    //-------------------------------INLINE Functions-------------------------------//

    static inline double ScaleSensor(double value, double a, double b)
    {
        double result = value;
        if (a != 1 || b != 0)
        {
            result = value * a + b;
        }
        return result;
    };

    static inline double ScaleToRaw(double value, double a, double b)
    {
        double result = value;
        if (a != 1 || b != 0)
        {
            result = (value - b) / a;
        }
        return result;
    };

    static inline double ScaleToDouble(double value, double a, double b)
    {
        double result = value;
        if (a != 1 || b != 0)
        {
            result = value * a + b;
        }
        return result;
    };

    static inline int8_t ScaleAndConvertInt8(double value, double a, double b)
    {
        double result = ScaleToRaw(value, a, b);
        if (result < INT8_MIN || result > INT8_MAX)
        {
            throw std::runtime_error(
                    "Channel - ScaleAndConvertInt8: result exceeds int8_t value range, actual value: " +
                    std::to_string(result));
        }
        return (int8_t) result;
    };

    static inline int16_t ScaleAndConvertInt16(double value, double a, double b)
    {
        double result = ScaleToRaw(value, a, b);
        if (result < INT16_MIN || result > INT16_MAX)
        {
            throw std::runtime_error(
                    "Channel - ScaleAndConvertInt16: result exceeds int16_t value range, actual value: " +
                    std::to_string(result));
        }
        return (int16_t) result;
    };

    static inline int32_t ScaleAndConvertInt32(double value, double a, double b)
    {
        double result = ScaleToRaw(value, a, b);
        if (result < INT32_MIN || result > INT32_MAX)
        {
            throw std::runtime_error(
                    "Channel - ScaleAndConvertInt32: result exceeds int32_t value range, actual value: " +
                    std::to_string(result));
        }
        return (int32_t) result;
    };

    inline std::string GetStatePrefix()
    {
        return this->channelName + ":";
    }

    inline void SetState(const std::string &stateName, const double &value, uint64_t &timestamp)
    {
        std::string prefix = GetStatePrefix();
        StateController *controller = StateController::Instance();
        //set new state value
        controller->SetState(prefix + stateName, value, timestamp);
    }

    //-------------------------------RECEIVE Functions-------------------------------//

    template<typename VAR>
    void GetSetVariableResponse(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp,
                                const std::map<VAR, std::string> &variableMap,
                                const std::map<std::string, std::vector<double>> &scalingMap)
    {
        SetMsg_t *setMsg = (SetMsg_t *) canMsg->bit.data.uint8;
        std::string variableStateName = variableMap.at((VAR)(setMsg->variable_id));

        Debug::info("Received variable %s of channel %s", variableStateName.c_str(), channelName.c_str());
        //convert and scale
        std::vector<double> scalingParams = scalingMap.at(variableStateName);
        double value = ScaleToDouble((double)(setMsg->value), scalingParams[0], scalingParams[1]);

        //set new state value
        SetState(variableStateName, value, timestamp);
    };

    virtual void StatusResponse(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp);

    virtual void ResetSettingsResponse(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp);

    //-------------------------------SEND Functions-------------------------------//

    virtual void SendStandardCommand(uint8_t nodeID, uint8_t cmdID, uint8_t *command, uint32_t commandLength,
                                     uint8_t canBusChannelID, CANDriver *driver, bool testOnly);

    virtual void SetVariable(uint8_t cmdID, uint8_t nodeID, uint8_t variableID,
                             std::vector<double> &sensorScalingParams, std::vector<double> &params,
                             uint8_t canBusChannelID, CANDriver *driver, bool testOnly);

    virtual void GetVariable(uint8_t cmdID, uint8_t nodeID, uint8_t variableID, std::vector<double> &params,
                             uint8_t canBusChannelID, CANDriver *driver, bool testOnly);

    virtual void SendNoPayloadCommand(std::vector<double> &params, uint8_t nodeID, uint8_t cmdID,
                                      uint8_t canBusChannelID, CANDriver *driver, bool testOnly);

public:
    Channel(std::string channelTypeName, uint8_t channelID, std::string channelName, std::vector<double> sensorScaling, Channel *parent, uint8_t typeSize = 0) :
            channelTypeName(channelTypeName), channelID(channelID), channelName(std::move(channelName)), sensorScaling(sensorScaling), typeSize(typeSize)
    {
        commandMap = std::map<std::string, command_t>();
    };

    virtual ~Channel()
    {};

    //-------------------------------GETTER & SETTER Functions-------------------------------//

    virtual const std::string GetChannelTypeName();

    virtual void GetSensorValue(uint8_t *valuePtr, uint8_t &valueLength, double &value);

    virtual uint8_t GetChannelID()
    { return this->channelID; };

    virtual std::string GetChannelName()
    { return this->channelName; };

    virtual std::vector<double> GetScaling()
    { std::lock_guard<std::mutex> lock(scalingMtx); return this->sensorScaling; };

    virtual void SetScaling(std::vector<double> &sensorScaling)
    { std::lock_guard<std::mutex> lock(scalingMtx); this->sensorScaling = sensorScaling; };

    virtual std::vector<std::string> GetStates();

    virtual std::map<std::string, command_t> GetCommands();

    virtual std::string GetSensorName()
    { return GetStatePrefix() + "sensor"; };

    //-------------------------------RECEIVE Functions-------------------------------//

    virtual void ProcessCANCommand(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp)
    { throw std::logic_error("Channel - ProcessCANCommand: not implemented"); };

    //-------------------------------SEND Functions-------------------------------//

    virtual void RequestStatus(std::vector<double> &params, bool testOnly)
    { throw std::logic_error("Channel - RequestStatus: not implemented"); };

    virtual void RequestResetSettings(std::vector<double> &params, bool testOnly)
    { throw std::logic_error("Channel - RequestResetSettings: not implemented"); };

    //-------------------------------Utility Functions-------------------------------//

    virtual std::vector<double> ResetSensorOffset(std::vector<double> &params, bool testOnly);

    virtual void RequestCurrentState()
    { throw std::logic_error("Channel - RequestCurrentState: not implemented"); };
};

#endif // LLSERVER_ECUI_HOUBOLT_CHANNEL_H
