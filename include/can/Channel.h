#ifndef LLSERVER_ECUI_HOUBOLT_CHANNEL_H
#define LLSERVER_ECUI_HOUBOLT_CHANNEL_H

#include <stdint.h>
#include <string>
#include <cstdarg>
#include <map>

#include "can/Node.h"

class Channel {
protected:
    uint8_t channelID;
	Node* parent;
    std::map<uint8_t, std::string> commands;
    static const std::vector<std::string> states;
    static const std::map<std::string, std::function<void(...)> commandsMap;
public:
    Channel(uint8_t channelID, Node* parent) : channelID(channelID), parent(parent);
    virtual ~Channel() {};

    virtual void ProcessCANCommand(uint32* msg, size_t msgSize);
    uint8_t GetChannelID() {return this->channelID;};


    virtual std::vector<std::string> GetStates();
	virtual std::vector<std::string> GetCommands();

};

#endif; // LLSERVER_ECUI_HOUBOLT_CHANNEL_H