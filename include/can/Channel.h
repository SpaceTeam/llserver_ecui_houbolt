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

#include "can/CANDriver.h"
#include "can_houbolt/cmds.h"

class Channel {
private:
    //TODO: MP check if this is the only and correct way to implement static const with inheritation
    static const std::vector<std::string> states;
    static const std::map<std::string, std::vector<double>> sensorScalingMap;

protected:

    uint8_t channelID;
    const std::string channelName;
    double sensorScaling;
    const uint8_t typeSize; //in bytes
	Channel *parent; //Should only be from node type

	std::map<std::string, std::function<void(std::vector<double> &, bool)>> commandMap;

	static inline int32_t ScaleAndConvert(double value, double a, double b);

	virtual void SetVariable(uint8_t cmdID, uint8_t nodeID, uint8_t variableID, std::vector<double> &sensorScalingParams, std::vector<double> &params, uint8_t canBusChannelID, CANDriver *driver, bool testOnly);
	virtual void GetVariable(uint8_t cmdID, uint8_t nodeID, uint8_t variableID, std::vector<double> &params, uint8_t canBusChannelID, CANDriver *driver, bool testOnly);


	virtual void SendStandardCommand(uint8_t nodeID, uint8_t cmdID, uint8_t *command, uint32_t commandLength, uint8_t canBusChannelID, CANDriver *driver, bool testOnly);

public:
    Channel(uint8_t channelID, std::string channelName, double sensorScaling, Channel *parent, uint8_t typeSize=0) :
        channelID(channelID), channelName(std::move(channelName)), sensorScaling(sensorScaling), parent(parent), typeSize(typeSize)
    {
        commandMap = std::map<std::string, std::function<void(std::vector<double> &, bool)>>();
    };
    virtual ~Channel() {};

    virtual void ProcessCANCommand(uint32_t *msg, size_t msgSize) {throw std::logic_error("Channel - ProcessCANCommand: not implemented");};
    virtual void GetSensorValue(uint8_t *valuePtr, uint8_t &valueLength, double &value);
    virtual uint8_t GetChannelID() {return this->channelID;};
    virtual std::string GetChannelName() {return this->channelName;};
    virtual void SetScaling(double) {this->sensorScaling = sensorScaling;};

    /**
     * returns of the channel and combines it with channelName to a map
     * @return states of channel, sends a can command for each state, process can command writes the result to the
     * state controller
     */
    virtual std::vector<std::string> GetStates()
    {
        return states;
    };
	virtual std::map<std::string, std::function<void(std::vector<double> &, bool)>> GetCommands()
    {
        return commandMap;
    };

	virtual std::string GetSensorName() {return channelName + ":sensor";};

};

#endif // LLSERVER_ECUI_HOUBOLT_CHANNEL_H