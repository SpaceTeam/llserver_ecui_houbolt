#include "can/Node.h"

#include <map>
#include "can_houbolt/generic_cmds.h"
#include "can/DigitalOut.h"
#include "can/ADC16.h"
#include "can/ADC24.h"
#include "can/Servo.h"

/**
 * consider putting event mapping into llinterface
 * @param id
 * @param nodeInfo
 * @param driver
 */

Node::Node(uint8_t id, NodeInfoMsg_t& nodeInfo, CANDriver* driver)
{
	this->id = id;
	this->driver = driver;
	InitChannels(nodeInfo);
}

bool Node::InitChannels(NodeInfoMsg_t& nodeInfo)
{
    bool success = true;
	for (uint8_t i = 0; i < sizeof(nodeInfo.channel_type)/sizeof(uint8_t); i++)
	{
		CHANNEL_TYPE channelType = (CHANNEL_TYPE) nodeInfo.channel_type[i];
		Channel* ch = nullptr;

		switch(channelType)
		{
			case CHANNEL_TYPE_ADC16:
				ch = new ADC16(i);
			break;
//			case CHANNEL_TYPE_ADC16_SINGLE:
//				ch = new Adc16_single_t;
//			break;
			case CHANNEL_TYPE_ADC24:
				ch = new ADC24(i);
			break;
			case CHANNEL_TYPE_DIGITAL_OUT:
				ch = new DigitalOut(i);
			break;
			case CHANNEL_TYPE_SERVO:
				ch = new Servo(i);
			break;
		    default:
		        success = false;
		        Debug::error("channel type not recognized");
			    // TODO: default case for unknown channel types that logs (DB)
		}

		if (ch != nullptr)
		{
			channelMap[i] = ch;
		}
	}
	return success;
}

Node::~Node()
{
    for (auto& ch: channelMap)
    {
        delete ch.second;
    }
}
