#include "can/Node.h"

#include <map>
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

//

Node::Node(uint8_t nodeID, std::string nodeChannelName, NodeInfoMsg_t& nodeInfo, std::map<uint8_t, std::tuple<std::string, double>> &channelInfo, CANDriver *driver)
    : nodeID(nodeID), driver(driver), Channel::Channel(0xFF, nodeChannelName, 1.0, this)
{
    InitChannels(nodeInfo, channelInfo);
}

void Node::InitChannels(NodeInfoMsg_t &nodeInfo, std::map<uint8_t, std::tuple<std::string, double>> &channelInfo)
{
	for (uint8_t i = 0; i < 32; i++)
    {
        uint32_t mask = 0x00000001 & (nodeInfo.channel_mask >> i);
        if (mask == 1)
        {
            auto channelType = (CHANNEL_TYPE) nodeInfo.channel_type[i];
            Channel* ch = nullptr;

            switch(channelType)
            {
                case CHANNEL_TYPE_ADC16:
                    ch = new ADC16(i, std::get<0>(channelInfo[i]), std::get<1>(channelInfo[i]), this);
                break;
    //			case CHANNEL_TYPE_ADC16_SINGLE:
    //				ch = new Adc16_single_t;
    //			break;
                case CHANNEL_TYPE_ADC24:
                    ch = new ADC24(i, std::get<0>(channelInfo[i]), std::get<1>(channelInfo[i]), this);
                break;
                case CHANNEL_TYPE_DIGITAL_OUT:
                    ch = new DigitalOut(i, std::get<0>(channelInfo[i]), std::get<1>(channelInfo[i]), this);
                break;
                case CHANNEL_TYPE_SERVO:
                    ch = new Servo(i, std::get<0>(channelInfo[i]), std::get<1>(channelInfo[i]), this);
                break;
                default:
                    throw std::runtime_error("channel type not recognized");
                    // TODO: default case for unknown channel types that logs (DB)
            }

            channelMap[i] = ch;
        }
        else if (mask > 1)
        {
            throw std::runtime_error("Node - InitChannels: mask convertion of node info failed");
        }
	}
}

Node::~Node()
{
    delete &channelMap;
}
