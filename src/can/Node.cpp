#include "can/Node.h"

#include <map>
#include <string>
#include <functional>
#include "can/DigitalOut.h"
#include "can/ADC16.h"
#include "can/ADC24.h"
#include "can/Servo.h"


const std::vector<std::string> Node::states = {"Bus1Voltage", "Bus2Voltage", "PowerVoltage", "PowerCurrent", "RefreshDivider", "RefreshRate"};
const std::map<std::string, std::vector<double>> Node::scalingMap =
        {
            {"Bus1Voltage", {1.0, 0.0}},
            {"Bus2Voltage", {1.0, 0.0}},
            {"PowerVoltage", {1.0, 0.0}},
            {"PowerCurrent", {1.0, 0.0}},
            {"RefreshDivider", {1.0, 0.0}},
            {"RefreshRate", {1.0, 0.0}}
        };

/**
 * consider putting event mapping into llinterface
 * @param id
 * @param nodeInfo
 * @param driver
 */

Node::Node(uint8_t nodeID, std::string nodeChannelName, NodeInfoMsg_t& nodeInfo, std::map<uint8_t, std::tuple<std::string, double>> &channelInfo, uint8_t canBusChannelID, CANDriver *driver)
    : nodeID(nodeID), driver(driver), Channel::Channel(0xFF, nodeChannelName, 1.0, this)
{
    commandMap = {
        {"SetBus1Voltage", std::bind(&Node::SetBus1Voltage, this, std::placeholders::_1, std::placeholders::_2)},
        {"GetBus1Voltage", std::bind(&Node::GetBus1Voltage, this, std::placeholders::_1, std::placeholders::_2)},
        {"SetBus2Voltage", std::bind(&Node::SetBus2Voltage, this, std::placeholders::_1, std::placeholders::_2)},
        {"GetBus2Voltage", std::bind(&Node::GetBus2Voltage, this, std::placeholders::_1, std::placeholders::_2)},
        {"SetPowerVoltage", std::bind(&Node::SetPowerVoltage, this, std::placeholders::_1, std::placeholders::_2)},
        {"GetPowerVoltage", std::bind(&Node::GetPowerVoltage, this, std::placeholders::_1, std::placeholders::_2)},
        {"SetPowerCurrent", std::bind(&Node::SetPowerCurrent, this, std::placeholders::_1, std::placeholders::_2)},
        {"GetPowerCurrent", std::bind(&Node::GetPowerCurrent, this, std::placeholders::_1, std::placeholders::_2)},
        {"SetRefreshDivider", std::bind(&Node::SetRefreshDivider, this, std::placeholders::_1, std::placeholders::_2)},
        {"GetRefreshDivider", std::bind(&Node::GetRefreshDivider, this, std::placeholders::_1, std::placeholders::_2)},
        {"SetRefreshTime", std::bind(&Node::SetRefreshRate, this, std::placeholders::_1, std::placeholders::_2)},
        {"GetRefreshTime", std::bind(&Node::GetRefreshRate, this, std::placeholders::_1, std::placeholders::_2)}
    };

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

std::map<std::string, std::function<void(std::vector<double> &, bool)>> Node::GetCommands()
{
    std::map<std::string, std::function<void(std::vector<double> &, bool)>> commands;
    commands.insert(Node::commandMap.begin(), Node::commandMap.end());
    for (auto &channel : channelMap)
    {
        std::map<std::string, std::function<void(std::vector<double> &, bool)>> chCommands = channel.second->GetCommands();
        commands.insert(chCommands.begin(), chCommands.end());
    }
    return commands;
}

void Node::SetBus1Voltage(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("Bus1Voltage");
    try
    {
        SetVariable(GENERIC_REQ_SET_VARIABLE, this->nodeID, GENERIC_BUS1_VOLTAGE, scalingParams, params, this->canBusChannelID, driver, testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Node - SetBus1Voltage: " + std::string(e.what()));
    }
}

void Node::GetBus1Voltage(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(GENERIC_REQ_GET_VARIABLE, this->nodeID, GENERIC_BUS1_VOLTAGE, params, this->canBusChannelID, driver, testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Node - GetBus1Voltage: " + std::string(e.what()));
    }
}

void Node::SetBus2Voltage(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("Bus2Voltage");
    try
    {
        SetVariable(GENERIC_REQ_SET_VARIABLE, this->nodeID, GENERIC_BUS2_VOLTAGE, scalingParams, params, this->canBusChannelID, driver, testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Node - SetBus2Voltage: " + std::string(e.what()));
    }
}

void Node::GetBus2Voltage(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(GENERIC_REQ_GET_VARIABLE, this->nodeID, GENERIC_BUS2_VOLTAGE, params, this->canBusChannelID, driver, testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Node - GetBus2Voltage: " + std::string(e.what()));
    }
}

void Node::SetPowerVoltage(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("PowerVoltage");
    try
    {
        SetVariable(GENERIC_REQ_SET_VARIABLE, this->nodeID, GENERIC_PWR_VOLTAGE, scalingParams, params, this->canBusChannelID, driver, testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Node - SetPowerVoltage: " + std::string(e.what()));
    }
}

void Node::GetPowerVoltage(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(GENERIC_REQ_GET_VARIABLE, this->nodeID, GENERIC_PWR_VOLTAGE, params, this->canBusChannelID, driver, testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Node - GetPowerVoltage: " + std::string(e.what()));
    }
}

void Node::SetPowerCurrent(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("PowerCurrent");
    try
    {
        SetVariable(GENERIC_REQ_SET_VARIABLE, this->nodeID, GENERIC_PWR_CURRENT, scalingParams, params, this->canBusChannelID, driver, testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Node - SetPowerCurrent: " + std::string(e.what()));
    }
}

void Node::GetPowerCurrent(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(GENERIC_REQ_GET_VARIABLE, this->nodeID, GENERIC_PWR_CURRENT, params, this->canBusChannelID, driver, testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Node - GetPowerCurrent: " + std::string(e.what()));
    }
}

void Node::SetRefreshDivider(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("RefreshDivider");
    try
    {
        SetVariable(GENERIC_REQ_SET_VARIABLE, this->nodeID, GENERIC_REFRESH_DIVIDER, scalingParams, params, this->canBusChannelID, driver, testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Node - SetRefreshDivider: " + std::string(e.what()));
    }
}

void Node::GetRefreshDivider(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(GENERIC_REQ_GET_VARIABLE, this->nodeID, GENERIC_REFRESH_DIVIDER, params, this->canBusChannelID, driver, testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Node - GetRefreshDivider: " + std::string(e.what()));
    }
}

void Node::SetRefreshRate(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("RefreshRate");
    try
    {
        SetVariable(GENERIC_REQ_SET_VARIABLE, this->nodeID, GENERIC_REFRESH_RATE, scalingParams, params, this->canBusChannelID, driver, testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Node - SetRefreshRate: " + std::string(e.what()));
    }
}

void Node::GetRefreshRate(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(GENERIC_REQ_GET_VARIABLE, this->nodeID, GENERIC_REFRESH_RATE, params, this->canBusChannelID, driver, testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Node - GetRefreshRate: " + std::string(e.what()));
    }
}