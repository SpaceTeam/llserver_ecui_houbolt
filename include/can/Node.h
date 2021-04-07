//
// Created by Markus on 31.03.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_NODE_H
#define LLSERVER_ECUI_HOUBOLT_NODE_H

#include "common.h"
#include "can/Channel.h"
#include "can/CANDriver.h"
#include "can_houbolt/generic_cmds.h"

class Node : public Channel
{
private:
    uint8_t canBusChannelID;
	uint8_t nodeID;
	std::map<uint8_t, Channel *> channelMap;
    CANDriver* driver;

	void InitChannels(NodeInfoMsg_t &nodeInfo, std::map<uint8_t, std::tuple<std::string, double>> &channelInfo);

public:
    //TODO: MP consider if putting channelid as parameter is necessary adapt initializer list if so
	Node(uint8_t nodeID, std::string nodeChannelName, NodeInfoMsg_t &nodeInfo, std::map<uint8_t, std::tuple<std::string, double>> &channelInfo, uint8_t canBusChannelID, CANDriver *driver);
	~Node();

	uint8_t GetNodeID();

    std::vector<std::string> GetStates() override;
};

#endif //LLSERVER_ECUI_HOUBOLT_NODE_H
