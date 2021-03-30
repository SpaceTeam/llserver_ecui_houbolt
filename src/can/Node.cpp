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
		CHANNEL_TYPE channelType = (CHANNEL_TYPE) nodeInfo.channel_type[i];
		Channel ch = NULL;

		switch(channelType)
		{
			case CHANNEL_TYPE_ADC16:
				ch = new Adc16_t;
			break;
			case CHANNEL_TYPE_ADC16_SINGLE:
				ch = new Adc16_single_t;
			break;
			case CHANNEL_TYPE_ADC24:
				ch = new Adc24_Channel(i);
			break;
			case CHANNEL_TYPE_DIGITAL_OUT:
				ch = new Digital_out_t;
			break;
			case CHANNEL_TYPE_SERVO:
				ch = Servo_t;
			break;
			// TODO: default case for unknown channel types that logs (DB)
		}

		if(channel != NULL){
			channelMap.insert(i, ch);
		}
	}
}

Node::~Node()
{
	delete channelMap;
}
