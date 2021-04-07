#ifndef LLSERVER_ECUI_HOUBOLT_CHANNEL_H
#define LLSERVER_ECUI_HOUBOLT_CHANNEL_H

#include "common.h"

#include <stdint.h>
#include <string>
#include <cstdarg>
#include <map>
#include <utility>

class Channel {
protected:

    uint8_t channelID;
    const std::string channelName;
    double scaling;
	Channel *parent; //Should only be from node type

    static const std::vector<std::string> states;
    static const std::map<std::string, std::function<void(std::vector<double>)>> commandsMap;
public:
    Channel(uint8_t channelID, std::string channelName, double scaling, Channel *parent) :
        channelID(channelID), channelName(std::move(channelName)), scaling(scaling), parent(parent) {};
    virtual ~Channel() {};

    virtual void ProcessCANCommand(uint32_t *msg, size_t msgSize);
    virtual uint8_t GetChannelID() {return this->channelID;};
    virtual std::string GetChannelName() {return this->channelName;};
    virtual void SetScaling(double) {this->scaling = scaling;};

    /**
     * fetches each value for each state of the channel and combines it with channelName to a map
     * @return
     */
    virtual std::map<std::string, std::tuple<double, uint64_t>> GetStates();
    virtual std::vector<std::string> GetStateNames();
	virtual std::map<std::string, std::function<void(std::vector<double>)>> GetCommands();

	virtual std::string GetSensorName() {return channelName + ":sensor";};

};

#endif; // LLSERVER_ECUI_HOUBOLT_CHANNEL_H