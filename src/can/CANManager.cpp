//
// Created by Markus on 03.04.21.
//

#include <chrono>
#include <thread>
#include <utility>
#include <bus_params_tq.h>

#include "utility/Config.h"
#include "can/CANManager.h"
#include "can_houbolt/channels/generic_channel_def.h"

#include "StateController.h"
#include "EventManager.h"

CANManager::~CANManager()
{
    delete mapping;
    delete canDriver;
}

bool CANManager::IsInitialized()
{
    return initialized;
}

//TODO: MP maybe move to node class
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

        try
        {
            std::string mappingPath = std::get<std::string>(Config::getData("mapping_path"));
            mapping = new CANMapping(mappingPath, "CANMapping");
            Debug::print("CANMapping initialized");

            Debug::print("Initializing CANDriver...");
            //arbitration bus parameters
            int32_t bitrate = std::get<int>(Config::getData("CAN/BUS/ARBITRATION/bitrate"));
            int32_t tseg1 = std::get<int>(Config::getData("CAN/BUS/ARBITRATION/time_segment_1"));
            int32_t tseg2 = std::get<int>(Config::getData("CAN/BUS/ARBITRATION/time_segment_2"));
            int32_t sjw = std::get<int>(Config::getData("CAN/BUS/ARBITRATION/sync_jump_width"));
            int32_t noSamp = std::get<int>(Config::getData("CAN/BUS/ARBITRATION/no_sampling_points"));

            CANParams arbitrationParams = {bitrate, tseg1, tseg2, sjw, noSamp};

            //data bus parameters
            int64_t bitrateData = std::get<int>(Config::getData("CAN/BUS/DATA/bitrate"));
            int32_t tseg1Data = std::get<int>(Config::getData("CAN/BUS/DATA/time_segment_1"));
            int32_t tseg2Data = std::get<int>(Config::getData("CAN/BUS/DATA/time_segment_2"));
            int32_t sjwData = std::get<int>(Config::getData("CAN/BUS/DATA/sync_jump_width"));

            CANParams dataParams = {bitrateData, tseg1Data, tseg2Data, sjwData};

            canDriver = new CANDriver(std::bind(&CANManager::OnCANInit, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                    std::bind(&CANManager::OnCANRecv, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                    std::bind(&CANManager::OnCANError, this, std::placeholders::_1), arbitrationParams, dataParams);

            Debug::print("Retreiving CANHardware info...");
            RequestCANInfo();
            using namespace std::chrono_literals;
            //TODO: wait for user input or expected node count to continue
            int32_t nodeCount = std::get<int>(Config::getData("CAN/node_count"));
            uint32_t currNodeCount = 0;
            do {
                std::this_thread::sleep_for(100ms);
                nodeMapMtx.lock();
                currNodeCount = nodeMap.size();
                nodeMapMtx.unlock();
                Debug::print("Waiting for nodes... %d of %d", currNodeCount, nodeCount);
            }
            while(currNodeCount < nodeCount);
            Debug::print("Initialized all nodes\n");
            canDriver->InitDone();


            initialized = true;
        }
        catch (std::exception& e)
        {
            throw std::runtime_error("Initializing CANManager failed: " + std::string(e.what()));
        }

    }
    else
    {
        Debug::error("CANManager already initialized");
        return CANResult::ERROR;
    }
    return CANResult::SUCCESS;
}

/**
 * only protocol message implemented inside CANManager
 * @return
 */
CANResult CANManager::RequestCANInfo()
{
    //TODO: MP change to correct broadcasting id
    Can_MessageId_t canID = {0};
    canID.info.direction = MASTER2NODE_DIRECTION;
    canID.info.node_id = 12;
    canID.info.special_cmd = STANDARD_SPECIAL_CMD;
    canID.info.priority = STANDARD_PRIORITY;

    Can_MessageData_t msg = {0};
    msg.bit.info.buffer = DIRECT_BUFFER;
    msg.bit.info.channel_id = GENERIC_CHANNEL_ID;
    msg.bit.cmd_id = GENERIC_REQ_NODE_INFO;

    uint32_t msgLength = sizeof(Can_MessageDataInfo_t) + sizeof(uint8_t);

    //TODO: MP be careful if one channel is used for backup
    canDriver->SendCANMessage(0, canID.uint32, msg.uint8, msgLength);
    canDriver->SendCANMessage(1, canID.uint32, msg.uint8, msgLength);
    canDriver->SendCANMessage(2, canID.uint32, msg.uint8, msgLength);
    //canDriver->SendCANMessage(3, canID.uint32, msg.uint8, msgLength);
}

/**
 * returns latest sensor data in map format
 * @return
 */
std::map<std::string, std::tuple<double, uint64_t>> CANManager::GetLatestSensorData()
{
    std::map<std::string, std::tuple<double, uint64_t>> latestSensorDataMap;
    std::map<std::string, std::tuple<double, uint64_t>> currentMap;
    Node *currNode;
    for (auto &it : nodeMap)
    {
        currNode = it.second;
        currentMap = currNode->GetLatestSensorData();
        latestSensorDataMap.insert(currentMap.begin(), currentMap.end());
    }
    return latestSensorDataMap;
}

void CANManager::OnChannelStateChanged(std::string stateName, double value, uint64_t timestamp)
{
    StateController *stateController = StateController::Instance();
    stateController->SetState(std::move(stateName), value, timestamp);
}

void CANManager::OnCANInit(uint8_t canBusChannelID, uint32_t canID, uint8_t *payload, uint32_t payloadLength, uint64_t timestamp)
{
    //TODO: only accept node info messages in this stage
    Can_MessageData_t *canMsg = (Can_MessageData_t *) payload;
    Can_MessageId_t *canIDStruct = (Can_MessageId_t *) (&canID);
    try
    {
        if (canMsg->bit.info.channel_id == GENERIC_CHANNEL_ID && canMsg->bit.cmd_id == GENERIC_RES_NODE_INFO)
        {
            uint8_t nodeID = canIDStruct->info.node_id;
            CANMappingObj nodeMappingObj = mapping->GetNodeObj(nodeID);

            NodeInfoMsg_t *nodeInfo = (NodeInfoMsg_t *) &canMsg->bit.data.uint8;

            std::map<uint8_t, std::tuple<std::string, double>> nodeChannelInfo;
            for (uint8_t channelID = 0; channelID < 32; channelID++)
            {
                uint32_t mask = 0x00000001 & (nodeInfo->channel_mask >> channelID);
                if (mask == 1)
                {
                    CANMappingObj channelMappingObj = mapping->GetChannelObj(nodeID, channelID);
                    nodeChannelInfo[channelID] = {channelMappingObj.stringID, channelMappingObj.scaling};

                    //add sensor names and scaling to array for fast sensor processing
                    uint16_t mergedID = MergeNodeIDAndChannelID(nodeID, channelID);
                    sensorInfoMap[mergedID] = {channelMappingObj.stringID, channelMappingObj.scaling};

                }
                else if (mask > 1)
                {
                    throw std::logic_error("CANManager - OnCANInit: mask conversion of node info failed");
                }
            }

            Node *node = new Node(nodeID, nodeMappingObj.stringID, *nodeInfo, nodeChannelInfo, canBusChannelID, canDriver);
            nodeMapMtx.lock();
            nodeMap[nodeID] = node;
            nodeMapMtx.unlock();

            //add states to state controller
            auto states = node->GetStates();
            StateController *stateController = StateController::Instance();
            stateController->AddUninitializedStates(states);


            //add available commands to event manager
            EventManager *eventManager = EventManager::Instance();
            eventManager->AddCommands(node->GetCommands());
        }
    }
    catch (std::runtime_error &e)
    {
        throw std::runtime_error("CANManager - OnCANInit: runtime error " + std::string(e.what()));
    }
    catch (std::logic_error &e)
    {
        throw std::logic_error("CANManager - OnCANInit: logic error " + std::string(e.what()));
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("CANManager - OnCANInit: other error  " + std::string(e.what()));
    }


}

void CANManager::OnCANRecv(uint8_t canBusChannelID, uint32_t canID, uint8_t *payload, uint32_t payloadLength, uint64_t timestamp)
{
    Can_MessageData_t *canMsg = (Can_MessageData_t *) payload;
    Can_MessageId_t *canIDStruct = (Can_MessageId_t *) (&canID);
    uint8_t nodeID = canIDStruct->info.node_id;

    try
    {
        //Don't require mutex at this point, since it is read only after initialization
        Node *node = nodeMap[nodeID];
        if (payloadLength <= 0)
        {
            throw std::runtime_error("CANManager - OnCANRecv: message with 0 payload not supported");
        }
        //TODO: move logic to node
        else if (canMsg->bit.info.channel_id == GENERIC_CHANNEL_ID && canMsg->bit.cmd_id == GENERIC_RES_DATA)
        {
            //TODO: remove when ringbuffer implemented
            RingBuffer<Sensor_t> buffer;
            node->ProcessSensorDataAndWriteToRingBuffer(canMsg, payloadLength, timestamp, buffer);
        }
        else
        {
            node->ProcessCANCommand(canMsg, payloadLength, timestamp);
        }
    }
    catch (std::runtime_error &e)
    {
        throw std::runtime_error("CANManager - OnCANRecv: runtime error " + std::string(e.what()));
    }
    catch (std::logic_error &e)
    {
        throw std::logic_error("CANManager - OnCANRecv: logic error " + std::string(e.what()));
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("CANManager - OnCANRecv: other error  " + std::string(e.what()));
    }


}

void CANManager::OnCANError(std::string *error)
{
    Debug::error("CANManager - OnCANError: CAN error %s", error->c_str());
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
