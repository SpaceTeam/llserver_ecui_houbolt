#include "../../include/can/Node.h"

#include <map>
#include <cstring>
#include <string>
#include <functional>
#include <utility>
#include "../../include/utility/Config.h"
#include "../../include/can/channels/DigitalOut.h"
#include "../../include/can/channels/ADC16.h"
#include "../../include/can/channels/ADC16Single.h"
#include "../../include/can/channels/ADC24.h"
#include "../../include/can/channels/DATA32.h"
#include "../../include/can/channels/Servo.h"
#include "../../include/can/channels/PneumaticValve.h"
#include "../../include/can/channels/Control.h"
#include "../../include/can/channels/PIControl.h"
#include "../../include/can/channels/IMU.h"
#include "../../include/can/channels/Rocket.h"
#include "../../include/StateController.h"
#include <unistd.h>

#include "can/channels/CANMonitor.h"

const std::vector<std::string> Node::states =
        {
            "Bus1Voltage",
            "Bus2Voltage",
            "PowerVoltage",
            "PowerCurrent",
            "RefreshDivider",
            "RefreshRate",
            "UARTEnabled",
            "ResetAllSettings",
            "LoggingEnabled",
            "FlashStatus",
            "LoraEnabled"
        };

const std::map<std::string, std::vector<double>> Node::scalingMap =
        {
            {"Bus1Voltage", {1.0, 0.0}},
            {"Bus2Voltage", {1.0, 0.0}},
            {"PowerVoltage", {1.0, 0.0}},
            {"PowerCurrent", {1.0, 0.0}},
            {"RefreshDivider", {1.0, 0.0}},
            {"RefreshRate", {1.0, 0.0}},
            {"UARTEnabled", {1.0, 0.0}},
            {"SpeakerToneFrequency", {1.0, 0.0}},
            {"SpeakerOnTime", {1.0, 0.0}},
            {"SpeakerOffTime", {1.0, 0.0}},
            {"SpeakerCount", {1.0, 0.0}},
            {"LoggingEnabled", {1.0, 0.0}},
            {"FlashStatus", {1.0, 0.0}},
            {"LoraEnabled", {1.0, 0.0}}
        };

const std::map<GENERIC_VARIABLES, std::string> Node::variableMap =
        {
            {GENERIC_BUS1_VOLTAGE, "Bus1Voltage"},
            {GENERIC_BUS2_VOLTAGE, "Bus2Voltage"},
            {GENERIC_PWR_VOLTAGE, "PowerVoltage"},
            {GENERIC_PWR_CURRENT, "PowerCurrent"},
            {GENERIC_REFRESH_DIVIDER, "RefreshDivider"},
            {GENERIC_REFRESH_RATE, "RefreshRate"},
            {GENERIC_UART_ENABLED, "UARTEnabled"},
            {GENERIC_LOGGING_ENABLED, "LoggingEnabled"},
            {GENERIC_LORA_ENABLED, "LoraEnabled"},
        };

InfluxDbLogger *Node::logger = nullptr;
bool Node::enableFastLogging;
std::string Node::influxIP;
int Node::influxPort;
std::string Node::databaseName;
std::string Node::measurementName;
int Node::influxBufferSize;

std::mutex Node::loggerMtx;

/**
 * consider putting event mapping into llinterface
 * @param id
 * @param nodeInfo
 * @param driver
 */

Node::Node(uint8_t nodeID, std::string nodeChannelName, NodeInfoMsg_t& nodeInfo, std::map<uint8_t, std::tuple<std::string, std::vector<double>>> &channelInfo, uint8_t canBusChannelID, CANDriver *driver)
    : Channel::Channel("Generic", 0xFF, std::move(nodeChannelName), {1.0, 0.0}, this), canBusChannelID(canBusChannelID), nodeID(nodeID), firwareVersion(nodeInfo.firmware_version), driver(driver)
{

    if (logger == nullptr)
    {
        Debug::info("%d, %s, %d, %s, %s, %d", enableFastLogging, influxIP.c_str(), influxPort, databaseName.c_str(), measurementName.c_str(), influxBufferSize);
#ifndef NO_INFLUX
        if (enableFastLogging)
        {
            Debug::print("Fast logging enabled");
            logger = new InfluxDbLogger();
            logger->Init(influxIP,
                        influxPort,
                        databaseName,
                        measurementName, MICROSECONDS,
                        influxBufferSize);
        }
        else
        {
#endif
            Debug::print("Fast logging disabled");
#ifndef NO_INFLUX
        }
#endif

    }

    commandMap = {
        {"SetBus1Voltage", {std::bind(&Node::SetBus1Voltage, this, std::placeholders::_1, std::placeholders::_2),{"Value"}}},
        {"GetBus1Voltage", {std::bind(&Node::GetBus1Voltage, this, std::placeholders::_1, std::placeholders::_2),{}}},
        {"SetBus2Voltage", {std::bind(&Node::SetBus2Voltage, this, std::placeholders::_1, std::placeholders::_2),{"Value"}}},
        {"GetBus2Voltage", {std::bind(&Node::GetBus2Voltage, this, std::placeholders::_1, std::placeholders::_2),{}}},
        {"SetPowerVoltage", {std::bind(&Node::SetPowerVoltage, this, std::placeholders::_1, std::placeholders::_2),{"Value"}}},
        {"GetPowerVoltage", {std::bind(&Node::GetPowerVoltage, this, std::placeholders::_1, std::placeholders::_2),{}}},
        {"SetPowerCurrent", {std::bind(&Node::SetPowerCurrent, this, std::placeholders::_1, std::placeholders::_2),{"Value"}}},
        {"GetPowerCurrent", {std::bind(&Node::GetPowerCurrent, this, std::placeholders::_1, std::placeholders::_2),{}}},
        {"SetRefreshDivider", {std::bind(&Node::SetRefreshDivider, this, std::placeholders::_1, std::placeholders::_2),{"Value"}}},
        {"GetRefreshDivider", {std::bind(&Node::GetRefreshDivider, this, std::placeholders::_1, std::placeholders::_2),{}}},
        {"SetRefreshTime", {std::bind(&Node::SetRefreshRate, this, std::placeholders::_1, std::placeholders::_2),{"Value"}}},
        {"GetRefreshTime", {std::bind(&Node::GetRefreshRate, this, std::placeholders::_1, std::placeholders::_2),{}}},
        {"SetUARTEnabled", {std::bind(&Node::SetUARTEnabled, this, std::placeholders::_1, std::placeholders::_2),{"Value"}}},
        {"GetUARTEnabled", {std::bind(&Node::GetUARTEnabled, this, std::placeholders::_1, std::placeholders::_2),{}}},
        {"SetLoggingEnabled", {std::bind(&Node::SetLoggingEnabled, this, std::placeholders::_1, std::placeholders::_2),{"Value"}}},
        {"GetLoggingEnabled", {std::bind(&Node::GetLoggingEnabled, this, std::placeholders::_1, std::placeholders::_2),{}}},
        {"SetLoraEnabled", {std::bind(&Node::SetLoraEnabled, this, std::placeholders::_1, std::placeholders::_2),{"Value"}}},
        {"GetLoraEnabled", {std::bind(&Node::GetLoraEnabled, this, std::placeholders::_1, std::placeholders::_2),{}}},
        {"RequestSetSpeaker", {std::bind(&Node::RequestSetSpeaker, this, std::placeholders::_1, std::placeholders::_2),{"ToneFrequency","OnTime","OffTime","Count"}}},
        {"RequestData", {std::bind(&Node::RequestData, this, std::placeholders::_1, std::placeholders::_2),{}}},
        {"RequestNodeStatus", {std::bind(&Node::RequestNodeStatus, this, std::placeholders::_1, std::placeholders::_2),{}}},
        {"RequestFlashClear", {std::bind(&Node::RequestFlashClear, this, std::placeholders::_1, std::placeholders::_2),{}}},
        {"RequestResetAllSettings", {std::bind(&Node::RequestResetAllSettings, this, std::placeholders::_1, std::placeholders::_2),{}}},
    };

    InitChannels(nodeInfo, channelInfo);
    //init latest sensor buffer with largest channel id
    latestSensorBufferLength = channelMap.rbegin()->first + 1;
    latestSensorBuffer = new SensorData_t[latestSensorBufferLength]{{0}};
}

void Node::InitConfig(Config &config) {
    enableFastLogging = config["/INFLUXDB/enable_fast_sensor_logging"];
    influxIP = config["/INFLUXDB/database_ip"];
    influxPort = config["/INFLUXDB/database_port"];
    databaseName = config["/INFLUXDB/database_name"];
    measurementName = config["/INFLUXDB/fast_sensor_measurement"];
    influxBufferSize = config["/INFLUXDB/fast_sensor_buffer_size"];
}

/**
 * might also throw exceptions from channelInfo, if channel id is not present
 * @param nodeInfo
 * @param channelInfo
 */
void Node::InitChannels(NodeInfoMsg_t &nodeInfo, std::map<uint8_t, std::tuple<std::string, std::vector<double>>> &channelInfo)
{
    uint8_t indexCounter = 0;
	for (uint8_t channelID = 0; channelID < 32; channelID++)
    {
        uint32_t mask = 0x00000001 & (nodeInfo.channel_mask >> channelID);
        if (mask == 1)
        {
            auto channelType = (CHANNEL_TYPE) nodeInfo.channel_type[indexCounter];
            Channel* ch = nullptr;

            switch(channelType)
            {
                case CHANNEL_TYPE_ADC16:
                    ch = new ADC16(channelID, std::get<0>(channelInfo[channelID]), std::get<1>(channelInfo[channelID]), this);
                    break;
    			case CHANNEL_TYPE_ADC16_SINGLE:
    				ch = new ADC16Single(channelID, std::get<0>(channelInfo[channelID]), std::get<1>(channelInfo[channelID]), this);
    			    break;
                case CHANNEL_TYPE_ADC24:
                    ch = new ADC24(channelID, std::get<0>(channelInfo[channelID]), std::get<1>(channelInfo[channelID]), this);
                    break;
                case CHANNEL_TYPE_DATA32:
                    ch = new DATA32(channelID, std::get<0>(channelInfo[channelID]), std::get<1>(channelInfo[channelID]), this);
                    break;
                case CHANNEL_TYPE_DIGITAL_OUT:
                    ch = new DigitalOut(channelID, std::get<0>(channelInfo[channelID]), std::get<1>(channelInfo[channelID]), this);
                    break;
                case CHANNEL_TYPE_SERVO:
                    ch = new Servo(channelID, std::get<0>(channelInfo[channelID]), std::get<1>(channelInfo[channelID]), this);
                    break;
                case CHANNEL_TYPE_PNEUMATIC_VALVE:
                    ch = new PneumaticValve(channelID, std::get<0>(channelInfo[channelID]), std::get<1>(channelInfo[channelID]), this);
                    break;
                case CHANNEL_TYPE_CONTROL:
                    ch = new Control(channelID, std::get<0>(channelInfo[channelID]), std::get<1>(channelInfo[channelID]), this);
                    break;
                case CHANNEL_TYPE_PI_CONTROL:
                    ch = new PIControl(channelID, std::get<0>(channelInfo[channelID]), std::get<1>(channelInfo[channelID]), this);
                    break;
                case CHANNEL_TYPE_IMU:
                    ch = new IMU(channelID, std::get<0>(channelInfo[channelID]), std::get<1>(channelInfo[channelID]), this);
                    break;
                case CHANNEL_TYPE_ROCKET:
                    ch = new Rocket(channelID, std::get<0>(channelInfo[channelID]), std::get<1>(channelInfo[channelID]), this);
                    break;
                case CHANNEL_TYPE_CAN_MONITOR:
                    ch = new CANMonitor(channelID, std::get<0>(channelInfo[channelID]), this);
                    break;
                default:
                    throw std::runtime_error("channel type not recognized");
                    // TODO: default case for unknown channel types that logs (DB)
            }

            channelMap[channelID] = ch;
            indexCounter++;
        }
        else if (mask > 1)
        {
            throw std::runtime_error("Node - InitChannels: mask convertion of node info failed");
        }
	}
}

Node::~Node()
{

}

//---------------------------------------------------------------------------------------//
//-------------------------------GETTER & SETTER Functions-------------------------------//
//---------------------------------------------------------------------------------------//

std::map<std::string, std::tuple<double, uint64_t>> Node::GetLatestSensorData()
{
    std::map<std::string, std::tuple<double, uint64_t>> sensorData;
    SensorData_t *copy = new SensorData_t[latestSensorBufferLength];
    size_t bytes = latestSensorBufferLength * sizeof(SensorData_t);

    bufferMtx.lock();
    std::memcpy(copy, latestSensorBuffer, bytes);
    bufferMtx.unlock();

    for (size_t i = 0; i < latestSensorBufferLength; i++)
    {
        if (channelMap.find(i) != channelMap.end())
        {

            sensorData[channelMap[i]->GetSensorName()] = {copy[i].value, copy[i].timestamp};
        }

    }
    /* if (nodeID==8)
    Debug::print("NodeID %d, %zd sensor data transmissions", nodeID, uint64_t(count));*/
    count = 0;
    return sensorData;
}

//TODO: add node name and channel names as prefix
std::vector<std::string> Node::GetStates()
{
    std::vector<std::string> states;
    states.insert(states.end(), Node::states.begin(), Node::states.end());

    //add node prefix to node specific states
    std::string prefix = GetStatePrefix();
    for (auto &state : states)
    {
        state.insert(0, prefix);
    }

    for (auto &channel : channelMap)
    {
        std::vector<std::string> chStates = channel.second->GetStates();

        states.insert(states.end(), chStates.begin(), chStates.end());
    }



    return states;
}

std::map<std::string, std::string> Node::GetChannelTypeMap()
{
    std::map<std::string, std::string> channelTypeMap;
    channelTypeMap[GetChannelName()] = GetChannelTypeName();

    for (auto &channel : channelMap)
    {
        channelTypeMap[channel.second->GetChannelName()] = channel.second->GetChannelTypeName();
    }

    return channelTypeMap;
}

std::map<std::string, command_t> Node::GetCommands()
{
    std::map<std::string, command_t> commandsTmp;
    std::map<std::string, command_t> commands;
    commandsTmp.insert(Node::commandMap.begin(), Node::commandMap.end());

    //add node prefix to node specific commands
    std::string prefix = GetStatePrefix();
    std::vector<void*> nhList;
    while (!commandsTmp.empty())
    {
        auto nodeHandle = commandsTmp.extract(commandsTmp.begin()->first);
        nodeHandle.key().insert(0, prefix);
        commands.insert(std::move(nodeHandle));
    }

    for (auto &channel : channelMap)
    {
        std::map<std::string, command_t> chCommands = channel.second->GetCommands();
        commands.insert(chCommands.begin(), chCommands.end());
    }

    return commands;
}

uint8_t Node::GetNodeID()
{
    return nodeID;
}

uint32_t Node::GetFirmwareVersion()
{
    return firwareVersion;
}

CANDriver *Node::GetCANDriver()
{
    return driver;
}

uint8_t Node::GetCANBusChannelID()
{
    return canBusChannelID;
}

//-------------------------------------------------------------------------------//
//-------------------------------RECEIVE Functions-------------------------------//
//-------------------------------------------------------------------------------//

//TODO: adapt to CanMessageData_t type
//TODO: add buffer writing
void Node::ProcessSensorDataAndWriteToRingBuffer(Can_MessageData_t *canMsg, uint32_t &canMsgLength,
                                                 uint64_t &timestamp)
{
    //TODO: make this more efficient
    if (canMsgLength < 2)
    {
        throw std::runtime_error("Node - ProcessSensorDataAndWriteToRingBuffer: payload length is smaller than 2, invalid can msg");
    }
    if (canMsg->bit.info.channel_id != GENERIC_CHANNEL_ID || canMsg->bit.cmd_id != GENERIC_RES_DATA)
    {
        throw std::runtime_error("Node - ProcessSensorDataAndWriteToRingBuffer: not a sensor data message, ignored...");
    }
    count++;

    SensorMsg_t *sensorMsg = (SensorMsg_t *) canMsg->bit.data.uint8;
    uint8_t *valuePtr = sensorMsg->channel_data;
    uint8_t currValueLength = 0;
    double currValue = 0;
    for (uint8_t channelID = 0; channelID < 32; channelID++)
    {
        uint32_t mask = 0x00000001 & (sensorMsg->channel_mask >> channelID);
        if (mask == 1)
        {
            Channel *ch;
            try
            {
                if (channelMap.find(channelID) == channelMap.end())
                {
                    throw std::runtime_error("Node - ProcessSensorDataAndWriteToRingBuffer: Channel not found");
                }
                ch = channelMap[channelID];

                ch->GetSensorValue(valuePtr, currValueLength, nameValueMap);

                // Debug::print("Hello chid %d, value %f", channelID, currValue);
                if (currValueLength <= 0)
                {
                    throw std::logic_error("Node - ProcessSensorDataAndWriteToRingBuffer: value length from channel is 0");
                }

                {
                    std::lock_guard lock(bufferMtx);

                    latestSensorBuffer[channelID] = {nameValueMap[0].second, timestamp};

#ifndef NO_INFLUX
                    if (enableFastLogging)
                    {
                        std::lock_guard loglock(loggerMtx);
                        for (auto it = nameValueMap.begin(); it != nameValueMap.end();) {
                            logger->log(it->first, it->second, timestamp);
                            nameValueMap.erase(it);
                        }

                        //logger->flush();
                    }
#endif
                    //buffer.push_back(sensor); //TODO: uncomment if implemented
                }

                valuePtr += currValueLength;
            }
            catch (std::exception &e)
            {
                throw std::runtime_error("Node - ProcessSensorDataAndWriteToRingBuffer: " + std::string(e.what()));
            }

        }
        else if (mask > 1)
        {
            throw std::logic_error("CANManager - OnCANInit: mask convertion of node info failed");
        }
    }
}

void Node::ProcessCANCommand(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp)
{
    try
    {
        if (canMsg->bit.info.channel_id != GENERIC_CHANNEL_ID)
        {
            if (channelMap.find(canMsg->bit.info.channel_id) == channelMap.end())
            {
                throw std::runtime_error("Node - ProcessSensorDataAndWriteToRingBuffer: Channel not found");
            }
            Channel *channel = channelMap[canMsg->bit.info.channel_id];
            channel->ProcessCANCommand(canMsg, canMsgLength, timestamp);
        }
        else
        {
            switch (canMsg->bit.cmd_id)
            {
                case GENERIC_RES_GET_VARIABLE:
                case GENERIC_RES_SET_VARIABLE:
                    GetSetVariableResponse<GENERIC_VARIABLES>(canMsg, canMsgLength, timestamp, variableMap, scalingMap);
                    break;
                case GENERIC_RES_NODE_STATUS:
                    NodeStatusResponse(canMsg, canMsgLength, timestamp);
                    break;
                case GENERIC_RES_RESET_ALL_SETTINGS:
                    ResetAllSettingsResponse(canMsg, canMsgLength, timestamp);
                    break;
                case GENERIC_RES_SYNC_CLOCK:
                    throw std::runtime_error("GENERIC_RES_SYNC_CLOCK: not implemented");
                    break;
                case GENERIC_RES_FLASH_STATUS:
                    FlashStatusResponse(canMsg, canMsgLength, timestamp);
                    break;
                case GENERIC_REQ_SET_VARIABLE:
                case GENERIC_REQ_GET_VARIABLE:
                case GENERIC_REQ_NODE_STATUS:
                case GENERIC_REQ_DATA:
                case GENERIC_REQ_SPEAKER:
                case GENERIC_REQ_NODE_INFO:
                case GENERIC_REQ_RESET_ALL_SETTINGS:
                case GENERIC_REQ_SYNC_CLOCK:
                case GENERIC_REQ_FLASH_CLEAR:
                    //TODO: uncomment after testing
                    //throw std::runtime_error("request message type has been received, major fault in protocol");
                    break;
                default:
                    throw std::runtime_error("node specific command with command id not supported: " + std::to_string(canMsg->bit.cmd_id));
            }
        }
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Node '" + this->channelName + "' - ProcessCANCommand: " + std::string(e.what()));
    }
}

void Node::NodeStatusResponse(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp)
{
    //NodeStatusMsg_t *statusMsg = (NodeStatusMsg_t *) canMsg->bit.data.uint8;

    throw std::logic_error("Node - NodeStatusResponse: not implemented");
}

void Node::ResetAllSettingsResponse(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp)
{
    SetState("ResetAllSettings", 1, timestamp);
}

void Node::FlashStatusResponse(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp)
{
    FlashStatusMsg_t *resMsg = (FlashStatusMsg_t *) canMsg->bit.data.uint8;
    SetState("FlashStatus", (double)resMsg->status, timestamp);
}


//----------------------------------------------------------------------------//
//-------------------------------SEND Functions-------------------------------//
//----------------------------------------------------------------------------//

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

void Node::SetUARTEnabled(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(GENERIC_REQ_GET_VARIABLE, this->nodeID, GENERIC_UART_ENABLED, params, this->canBusChannelID, driver, testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Node - SetUARTEnabled: " + std::string(e.what()));
    }
}

void Node::GetUARTEnabled(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(GENERIC_REQ_GET_VARIABLE, this->nodeID, GENERIC_UART_ENABLED, params, this->canBusChannelID, driver, testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Node - GetUARTEnabled: " + std::string(e.what()));
    }
}

void Node::SetLoggingEnabled(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("LoggingEnabled");
    try
    {
        SetVariable(GENERIC_REQ_SET_VARIABLE, this->nodeID, GENERIC_LOGGING_ENABLED, scalingParams, params, this->canBusChannelID, driver, testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Node - SetLoggingEnabled: " + std::string(e.what()));
    }
}

void Node::GetLoggingEnabled(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(GENERIC_REQ_GET_VARIABLE, this->nodeID, GENERIC_LOGGING_ENABLED, params, this->canBusChannelID, driver, testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Node - GetLoggingEnabled: " + std::string(e.what()));
    }
}

void Node::SetLoraEnabled(std::vector<double> &params, bool testOnly)
{
    std::vector<double> scalingParams = scalingMap.at("LoraEnabled");
    try
    {
        SetVariable(GENERIC_REQ_SET_VARIABLE, this->nodeID, GENERIC_LORA_ENABLED, scalingParams, params, this->canBusChannelID, driver, testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Node - SetLoraEnabled: " + std::string(e.what()));
    }
}

void Node::GetLoraEnabled(std::vector<double> &params, bool testOnly)
{
    try
    {
        GetVariable(GENERIC_REQ_GET_VARIABLE, this->nodeID, GENERIC_LORA_ENABLED, params, this->canBusChannelID, driver, testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Node - GetLoraEnabled: " + std::string(e.what()));
    }
}

void Node::RequestSetSpeaker(std::vector<double> &params, bool testOnly)
{
    try
    {
        if (params.size() != 4) //number of required parameters
        {
            throw std::runtime_error("4 parameters expected, but " + std::to_string(params.size()) + " were provided");
        }
        std::vector<double> scalingSpeakerToneFrequency = scalingMap.at("SpeakerToneFrequency");
        std::vector<double> scalingSpeakerOnTime = scalingMap.at("SpeakerOnTime");
        std::vector<double> scalingSpeakerOffTime = scalingMap.at("SpeakerOffTime");
        std::vector<double> scalingSpeakerCount = scalingMap.at("SpeakerCount");

        SpeakerMsg_t speakerMsg = {0};
        speakerMsg.tone_frequency = Channel::ScaleAndConvertInt16(params[0],scalingSpeakerToneFrequency[0],scalingSpeakerToneFrequency[1]);
        speakerMsg.on_time = Channel::ScaleAndConvertInt16(params[1],scalingSpeakerOnTime[0],scalingSpeakerOnTime[1]);
        speakerMsg.off_time = Channel::ScaleAndConvertInt16(params[2],scalingSpeakerOffTime[0],scalingSpeakerOffTime[1]);
        speakerMsg.count = Channel::ScaleAndConvertInt8(params[3],scalingSpeakerCount[0],scalingSpeakerCount[1]);

        SendStandardCommand(this->nodeID, GENERIC_REQ_SPEAKER, (uint8_t *) &speakerMsg, sizeof(speakerMsg), this->canBusChannelID, this->driver, testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Node - SetSpeaker: " + std::string(e.what()));
    }
}

void Node::RequestFlashClear(std::vector<double> &params, bool testOnly)
{
    try
    {
        if (params.size() != 0) //number of required parameters
        {
            throw std::runtime_error("0 parameter expected, but " + std::to_string(params.size()) + " were provided");
        }

        SendNoPayloadCommand(params, nodeID, GENERIC_REQ_FLASH_CLEAR, canBusChannelID, driver, testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Node - RequestFlashStatus: " + std::string(e.what()));
    }
}

void Node::RequestData(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, nodeID, GENERIC_REQ_DATA, canBusChannelID, driver, testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Node - RequestData: " + std::string(e.what()));
    }
}

void Node::RequestNodeStatus(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, nodeID, GENERIC_REQ_NODE_STATUS, canBusChannelID, driver, testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Node - RequestData: " + std::string(e.what()));
    }
}

void Node::RequestResetAllSettings(std::vector<double> &params, bool testOnly)
{
    try
    {
        SendNoPayloadCommand(params, nodeID, GENERIC_REQ_RESET_ALL_SETTINGS, canBusChannelID, driver, testOnly);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Node - RequestData: " + std::string(e.what()));
    }
}

//----------------------------------------------------------------------------//
//-----------------------------Utility Functions------------------------------//
//----------------------------------------------------------------------------//

void Node::FlushLogger()
{
    if (logger != nullptr)
    {
        logger->flush();
    }
}

std::vector<double> Node::ResetSensorOffset(std::vector<double> &params, bool testOnly)
{
    try
    {
        if (params.size() != 2) //number of required parameters
        {
            throw std::runtime_error("2 parameters expected (channelID, currValue), but " + std::to_string(params.size()) + " were provided");
        }
        uint8_t channelID = params[0];
        params.erase(params.begin());

        if (channelMap.find(channelID) == channelMap.end())
        {
            throw std::runtime_error("Node - ResetSensorOffset: Channel not found");
        }
        Channel *channel = channelMap[channelID];
        return channel->ResetSensorOffset(params, testOnly);
        


    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Node - ResetSensorOffset: " + std::string(e.what()));
    }
}

void Node::RequestCurrentState()
{
    std::vector<double> params;

    GetBus1Voltage(params, false);
	GetBus2Voltage(params, false);
	GetPowerVoltage(params, false);
	GetPowerCurrent(params, false);
	GetRefreshDivider(params, false);
	GetRefreshRate(params, false);
	GetUARTEnabled(params, false);
    GetLoggingEnabled(params, false);
    GetLoraEnabled(params, false);

    for (auto &channel : channelMap)
    {
        channel.second->RequestCurrentState();
        usleep(10000);
    }
}
