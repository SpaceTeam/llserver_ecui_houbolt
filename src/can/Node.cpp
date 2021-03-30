#include "Node.h"

#include <map>
#include "can_houbolt/generic_cmds.h"

class Node
{
private:
	uint8_t id;
	std::map<uint8_t, Channel> channelMap;
	InitChannels(NodeInfoMsg_t nodeInfo)

public:
	Node(uint8_t id, NodeInfoMsg_t nodeInfo);
	~Node();
};

Node::Node(uint8_t id, NodeInfoMsg_t nodeInfo)
{
	this->id = id;
	InitChannels(nodeInfo);
}

Node::InitChannels(NodeInfoMsg_t nodeInfo)
{
	for (int i = 0; i < sizeof(nodeInfo.channel_type)/sizeof(uint8_t); i++)
	{
		CHANNEL_TYPE channelType = nodeInfo.channel_type[i];
		switch(channelType)
		{
			case:
		}
	}
}

Node::~Node()
{
	delete channelMap;
}
