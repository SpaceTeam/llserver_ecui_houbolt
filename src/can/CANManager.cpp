//
// Created by Markus on 03.04.21.
//

#include <chrono>
#include <thread>
#include <utility>

#include "Config.h"
#include "can/CANManager.h"
#include "can_houbolt/generic_cmds.h"

#include "StateController.h"
#include "EventManager.h"

CANManager::~CANManager()
{
    delete &nodeMap;
}

inline uint8_t CANManager::GetNodeID(uint32_t &canID)
{
    return (uint8_t) ((0x0000008F & canID) >> 1);
}

inline uint16_t CANManager::MergeNodeIDAndChannelID(uint8_t &nodeId, uint8_t &channelId)
{
    return (uint16_t) ((nodeId << 8) | channelId);
}


CANResult CANManager::Init()
{
    if (!initialized)
    {
        Debug::print("Initializing CANMapping...");
        std::string mappingPath = std::get<std::string>(Config::getData("mapping_path"));
        mapping = new CANMapping(mappingPath, (std::string &) "CANMapping");
        Debug::print("CANMapping initialized");

        Debug::print("Initializing CANDriver...");
        //arbitration bus parameters
        =
        int32_t phase1 = std::get<int>(Config::getData("CAN/BUS/ARBITRATION/phase1"));
        int32_t phase2 = std::get<int>(Config::getData("CAN/BUS/ARBITRATION/phase2"));
        int32_t sjw = std::get<int>(Config::getData("CAN/BUS/ARBITRATION/sync_jump_width"));
        int32_t prop = std::get<int>(Config::getData("CAN/BUS/ARBITRATION/propagation_segment"));
        int32_t presc = std::get<int>(Config::getData("CAN/BUS/ARBITRATION/prescaler"));
        //data bus parameters
        int32_t tqData = std::get<int>(Config::getData("CAN/BUS/DATA/time_quanta"));
        int32_t phase1Data = std::get<int>(Config::getData("CAN/BUS/DATA/phase1"));
        int32_t phase2Data = std::get<int>(Config::getData("CAN/BUS/DATA/phase2"));
        int32_t sjwData = std::get<int>(Config::getData("CAN/BUS/DATA/sync_jump_width"));
        int32_t propData = std::get<int>(Config::getData("CAN/BUS/DATA/propagation_segment"));
        int32_t prescData = std::get<int>(Config::getData("CAN/BUS/DATA/prescaler"));
        canDriver = new CANDriver(std::bind(&CANManager::OnCANInit, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3),
                std::bind(&CANManager::OnCANRecv, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3),
                std::bind(&CANManager::OnCANError, this));

        Debug::print("Retreiving CANHardware info...");
        RequestCANInfo();
        using namespace std::chrono_literals;
        //TODO: wait for user input or expected node count to continue
        int32_t nodeCount = std::get<int>(Config::getData("CAN/node_count"));
        while( nodeCount)
        std::this_thread::sleep_for(std::chrono::duration<int, std::chrono::milliseconds>(1000));

        initialized = true;
    }
    else
    {
        Debug::error("CANManager already initialized");
        return CANResult::ERROR;
    }
    return CANResult::SUCCESS;
}

CANResult CANManager::RequestCANInfo()
{
    return CANResult::ERROR;
}

/**
 * returns latest sensor data in map format
 * locks the sensor buffer, copies it and unlocks it immediately afterwards so values can be updated again
 * try_lock is used on the other side, so no blocking occurs at time critical part
 * @return
 */
std::map<std::string, std::tuple<double, uint64_t>> CANManager::GetLatestSensorData()
{
    SensorData_t* latestSensorDataBufferCopy = new SensorData_t[sensorInfoMap.size()];
    std::map<std::string, std::tuple<double, uint64_t>> latestSensorDataMap;
    sensorMtx.lock();
    latestSensorDataBufferCopy = std::copy(latestSensorDataBuffer,
            latestSensorDataBuffer+latestSensorDataBufferLength,
            latestSensorDataBufferCopy);
    sensorMtx.unlock();
    for (SensorData_t *sensorData = latestSensorDataBufferCopy; sensorData < latestSensorDataBufferCopy + sensorInfoMap.size(); sensorData += sizeof(SensorData_t))
    {
        uint16_t key = (sensorData->nodeId << 8) | sensorData->channelId;

        std::tuple<std::string, double> value = sensorInfoMap[key];
        std::string name = std::get<0>(value);
        double scaling = std::get<1>(value);
        latestSensorDataMap[name] = {sensorData->data * scaling, sensorData->timestamp};
    }
    return latestSensorDataMap;
}

void CANManager::OnChannelStateChanged(std::string stateName, double value, uint64_t timestamp)
{
    StateController *stateController = StateController::Instance();
    stateController->ChangeState(std::move(stateName), value, timestamp);
}

void CANManager::OnCANInit(uint8_t canBusChannelID, uint32_t canID, uint8_t *payload, uint32_t payloadLength, uint64_t timestamp)
{
    //TODO: only accept node info messages in this stage
    if (payloadLength >= (sizeof(NodeInfoMsg_t)+1) && payload[1] == GENERIC_NODE_INFO)
    {
        uint8_t nodeID = CANManager::GetNodeID(canID);
        CANMappingObj nodeMappingObj = mapping->GetNodeObj(nodeID);

        NodeInfoMsg_t *nodeInfo = (NodeInfoMsg_t *)payload;

        std::map<uint8_t, std::tuple<std::string, double>> channelInfo;
        for (uint8_t channelID = 0; channelID < 32; channelID++)
        {
            uint32_t mask = 0x00000001 & (nodeInfo->channel_mask >> channelID);
            if (mask == 1)
            {
                CANMappingObj channelMappingObj = mapping->GetNodeObj(channelID);
                channelInfo[channelID] = {channelMappingObj.stringID, channelMappingObj.scaling};

                //add sensor names to array if needed
                uint16_t mergedID = MergeNodeIDAndChannelID(nodeID, channelID);
                sensorInfoMap[mergedID] = {channelMappingObj.stringID, channelMappingObj.scaling};
            }
            else if (mask > 1)
            {
                throw std::runtime_error("CANManager - OnCANInit: mask convertion of node info failed");
            }
        }

        Node *node = new Node(nodeID, nodeMappingObj.stringID, *nodeInfo, channelInfo, canBusChannelID, canDriver);
        nodeMap[nodeID] = node;

        //add states to state controller
        auto states = node->GetStates();
        StateController *stateController = StateController::Instance();
        stateController->AddUninitializedStates(states);


        //add available commands to event manager
        EventManager *eventManager = EventManager::Instance();
        eventManager->AddCommands(node->GetCommands());
    }
}

void CANManager::OnCANRecv(uint32_t canID, uint8_t *payload, uint32_t payloadLength, uint64_t timestamp)
{
    uint8_t nodeID = CANManager::GetNodeID(canID);
    try
    {
        Node *node = nodeMap[nodeID];
        //TODO: process can command inside node
    }
    catch(std::exception& e)
    {
        Debug::error("Node id or hlid do not exist: %s", e.what());
    }


}

void CANManager::OnCANError()
{
    Debug::error("CANManager - OnCANError: CAN error");
}

//std::vector<std::string> CANManager::GetChannelStates()
//{
//    if (initialized)
//    {
//        std::vector<std::string> stateNames;
//        for (auto &node : nodeMap)
//        {
//            std::vector<std::string> nodeStates;
//            nodeStates = node.second->GetStateNames();
//            stateNames.insert(stateNames.end(), nodeStates.begin(), nodeStates.end());
//        }
//
//    }
//    else
//    {
//        Debug::error("CANManager - GetChannelStates: CANManager not initialized");
//    }
//    return std::vector<std::string>();
//}
//
//std::map<std::string, std::function<CANResult(...)>> CANManager::GetChannelCommands()
//{
//    if (initialized)
//    {
//        std::map<std::string, std::function<CANResult(...)>> commands;
//        for (auto &node : nodeMap)
//        {
//            std::map<std::string, std::function<CANResult(...)>> nodeCommands = node.second->GetCommands();
//            const auto [it, success] = commands.insert(nodeCommands.begin(), nodeCommands.end());
//            if (!success)
//            {
//                Debug::error("CANManager - GetChannelCommands: insert failed");
//                return std::map<std::string, std::function<CANResult(...)>>();
//            }
//        }
//
//    }
//    else
//    {
//        Debug::error("CANManager - GetChannelCommands: CANManager not initialized");
//    }
//    return std::map<std::string, std::function<CANResult(...)>>();
//}
