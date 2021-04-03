//
// Created by Markus on 31.03.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_NODE_H
#define LLSERVER_ECUI_HOUBOLT_NODE_H

#include "common.h"
#include "Channel.h"
#include "CANDriver.h"

class Node : public Channel
{
private:
	uint8_t nodeID;
	std::map<uint8_t, Channel*> channelMap;
    CANDriver* driver;

	CANResult InitChannels(/*NodeInfoMsg_t& nodeInfo*/);

public:
	Node(uint8_t nodeID, /*NodeInfoMsg_t& nodeInfo,*/ CANDriver* driver) : nodeID(nodeID), driver(driver);
	~Node();

	uint8_t GetNodeID();

};

#endif //LLSERVER_ECUI_HOUBOLT_NODE_H
