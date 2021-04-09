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

Node::Node(uint8_t nodeID, std::string nodeChannelName, NodeInfoMsg_t& nodeInfo, std::map<uint8_t, std::tuple<std::string, double>> &channelInfo, uint8_t canBusChannelID, CANDriver *driver)
    : nodeID(nodeID), driver(driver), Channel::Channel(0xFF, nodeChannelName, 1.0, this)
{
    canBusChannelID = canBusChannelID;
    InitChannels(nodeInfo, channelInfo);
}

/**
 * might also throw exceptions from channelInfo, if channel id is not present
 * @param nodeInfo
 * @param channelInfo
 */
void Node::InitChannels(NodeInfoMsg_t &nodeInfo, std::map<uint8_t, std::tuple<std::string, double>> &channelInfo)
{
	for (uint8_t channelID = 0; channelID < 32; channelID++)
    {
        uint32_t mask = 0x00000001 & (nodeInfo.channel_mask >> channelID);
        if (mask == 1)
        {
            auto channelType = (CHANNEL_TYPE) nodeInfo.channel_type[channelID];
            Channel* ch = nullptr;

            switch(channelType)
            {
                case CHANNEL_TYPE_ADC16:
                    ch = new ADC16(channelID, std::get<0>(channelInfo[channelID]), std::get<1>(channelInfo[channelID]), this);
                break;
    //			case CHANNEL_TYPE_ADC16_SINGLE:
    //				ch = new Adc16_single_t;
    //			break;
                case CHANNEL_TYPE_ADC24:
                    ch = new ADC24(channelID, std::get<0>(channelInfo[channelID]), std::get<1>(channelInfo[channelID]), this);
                break;
                case CHANNEL_TYPE_DIGITAL_OUT:
                    ch = new DigitalOut(channelID, std::get<0>(channelInfo[channelID]), std::get<1>(channelInfo[channelID]), this);
                break;
                case CHANNEL_TYPE_SERVO:
                    ch = new Servo(channelID, std::get<0>(channelInfo[channelID]), std::get<1>(channelInfo[channelID]), this);
                break;
                default:
                    throw std::runtime_error("channel type not recognized");
                    // TODO: default case for unknown channel types that logs (DB)
            }

            channelMap[channelID] = ch;
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

std::vector<std::string> Node::GetStates()
{
    std::vector<std::string> states;
    states.insert(states.end(), Node::states.begin(), Node::states.end());
    for (auto &channel : channelMap)
    {
        std::vector<std::string> chStates = channel.second->GetStates();
        states.insert(states.end(), chStates.begin(), chStates.end());
    }
    return states;
}

std::map<std::string, std::function<void(std::vector<double>)>> Node::GetCommands()
{
    std::map<std::string, std::function<void(std::vector<double>)>> commands;
    commands.insert(Node::commandMap.begin(), Node::commandMap.end());
    for (auto &channel : channelMap)
    {
        std::map<std::string, std::function<void(std::vector<double>)>> chCommands = channel.second->GetCommands();
        commands.insert(chCommands.begin(), chCommands.end());
    }
    return commands;
}

void Node::SetRefreshDivider(std::vector<double> &params)
{
    
}



const std::vector<std::string> Channel::states = {"RefreshDivider"};
std::map<std::string, std::function<void(std::vector<double>)>> Channel::commandMap = {
    {"SetRefreshDivider", },
    {"GetRefreshDivider", }
};