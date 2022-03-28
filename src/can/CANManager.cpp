//
// Created by Markus on 03.04.21.
//

#include <chrono>
#include <thread>
#include <future>
#include <utility>

#include "utility/Config.h"
#include "can/CANManager.h"
#include "can/CANDriverKvaser.h"
#include "can/CANDriverSocketCAN.h"
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

            std::string can_driver = std::get<std::string>(Config::getData("CAN/DRIVER"));

            if(can_driver == "Kvaser")
            {
				#ifdef NO_CANLIB
            	Debug::print("Can driver \"Kvaser\" specified in config but excluded by Cmake argument NO_CANLIB");
            	throw std::runtime_error("Can driver \"Kvaser\" specified in config but excluded by Cmake argument NO_CANLIB");
				#else
            	Debug::print("Using Kvaser CAN driver");
				canDriver = new CANDriverKvaser(std::bind(&CANManager::OnCANRecv,  this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
				                                std::bind(&CANManager::OnCANError, this, std::placeholders::_1));
				#endif
            }
            else if(can_driver == "SocketCAN")
			{
            	Debug::print("Using SocketCAN driver");
            	canDriver = new CANDriverSocketCAN(std::bind(&CANManager::OnCANRecv,  this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
				                                   std::bind(&CANManager::OnCANError, this, std::placeholders::_1));
			}
            else
            {
            	Debug::print("Can driver \"" + can_driver + "\" specified in config not found!");
            	throw std::runtime_error("Can driver \"" + can_driver + "\" specified in config not found!");
            }

            Debug::print("Retreiving CANHardware info...");
            RequestCANInfo();
            using namespace std::chrono_literals;
            //TODO: wait for user input or expected node count to continue
            uint32_t nodeCount = std::get<int>(Config::getData("CAN/node_count"));
            uint32_t currNodeCount = 0;

            bool canceled = false;
            std::future<bool> future = std::async([](){
                    std::cin.get();
                    return true;
                });
            do {
                Debug::print("Waiting for nodes %d of %d, press enter to continue...", currNodeCount, nodeCount);
                if (future.wait_for(100ms) == std::future_status::ready)
                    canceled = true;
                nodeMapMtx.lock();
                currNodeCount = nodeMap.size();
                nodeMapMtx.unlock();
            }
            while((currNodeCount < nodeCount) && !canceled);

            Debug::print("Check for version differences...\n");
            std::vector<uint32_t> versions;
            nodeMapMtx.lock();
            for (auto& node : nodeMap)
            {
                uint32_t currFirmwareVersion = node.second->GetFirmwareVersion();
                if (std::find(versions.begin(), versions.end(), currFirmwareVersion) == versions.end())
                {
                    Debug::print("Version: 0x%08x", currFirmwareVersion);
                    versions.push_back(currFirmwareVersion);
                }
            }
            if (versions.size() > 1)
            {
                Debug::error("Multiple Versions Found!");
            }
            nodeMapMtx.unlock();

            Debug::print("Initialized all nodes, press enter to continue...\n");
            // no need for std::cin.get(); here, since the thread needs an input to quit

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
    canID.info.node_id = 0;
    canID.info.special_cmd = STANDARD_SPECIAL_CMD;
    canID.info.priority = STANDARD_PRIORITY;

    Can_MessageData_t msg = {0};
    msg.bit.info.buffer = DIRECT_BUFFER;
    msg.bit.info.channel_id = GENERIC_CHANNEL_ID;
    msg.bit.cmd_id = GENERIC_REQ_NODE_INFO;

    uint32_t msgLength = sizeof(Can_MessageDataInfo_t) + sizeof(uint8_t);

    Debug::print("---Press enter to send node request---");
    std::cin.get();
    //TODO: MP be careful if one channel is used for backup
    canDriver->SendCANMessage(0, canID.uint32, msg.uint8, msgLength);
    canDriver->SendCANMessage(1, canID.uint32, msg.uint8, msgLength);
    canDriver->SendCANMessage(2, canID.uint32, msg.uint8, msgLength);
    //canDriver->SendCANMessage(3, canID.uint32, msg.uint8, msgLength);

	return CANResult::SUCCESS;
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

void CANManager::OnCANRecv(uint8_t canBusChannelID, uint32_t canID, uint8_t *payload, uint32_t payloadLength, uint64_t timestamp)
{
	if(!initialized) // TODO consolidate code from the two initialized/!initialized cases
	{
		//TODO: only accept node info messages in this stage
		Can_MessageData_t *canMsg = (Can_MessageData_t *) payload;
		Can_MessageId_t *canIDStruct = (Can_MessageId_t *) (&canID);
		try
		{
			if (canMsg->bit.info.channel_id == GENERIC_CHANNEL_ID && canMsg->bit.cmd_id == GENERIC_RES_NODE_INFO)
			{
				uint8_t nodeID = canIDStruct->info.node_id;

				nodeMapMtx.lock();
				bool found = nodeMap.find(nodeID) != nodeMap.end();
				nodeMapMtx.unlock();
				if (found)
				{
					std::runtime_error("Node already initialized, possible logic error on hardware or in software, ignoring node info msg...");
				}

				CANMappingObj nodeMappingObj = mapping->GetNodeObj(nodeID);

				NodeInfoMsg_t *nodeInfo = (NodeInfoMsg_t *) &canMsg->bit.data.uint8;

				std::map<uint8_t, std::tuple<std::string, std::vector<double>>> nodeChannelInfo;
				for (uint8_t channelID = 0; channelID < 32; channelID++)
				{
					uint32_t mask = 0x00000001 & (nodeInfo->channel_mask >> channelID);
					if (mask == 1)
					{
						CANMappingObj channelMappingObj = mapping->GetChannelObj(nodeID, channelID);
						nodeChannelInfo[channelID] = {channelMappingObj.stringID, {channelMappingObj.slope, channelMappingObj.offset}};

						//add sensor names and scaling to array for fast sensor processing
						uint16_t mergedID = MergeNodeIDAndChannelID(nodeID, channelID);
						sensorInfoMap[mergedID] = {channelMappingObj.stringID, {channelMappingObj.slope, channelMappingObj.offset}};

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

				Debug::print("Node %s with ID %d on CAN Bus %d detected\n\t\t\tfirmware version 0x%08x", node->GetChannelName().c_str(), node->GetNodeID(), canBusChannelID, node->GetFirmwareVersion());
			}
		}
		catch (std::runtime_error &e)
		{
			throw std::runtime_error("CANManager - OnCANRecv Init: runtime error " + std::string(e.what()));
		}
		catch (std::logic_error &e)
		{
			throw std::logic_error("CANManager - OnCANRecv Init: logic error " + std::string(e.what()));
		}
		catch (std::exception &e)
		{
			throw std::runtime_error("CANManager - OnCANRecv Init: other error  " + std::string(e.what()));
		}
	}
	else // initialized, normal operation
	{
		Can_MessageData_t *canMsg = (Can_MessageData_t *) payload;
		Can_MessageId_t *canIDStruct = (Can_MessageId_t *) (&canID);
		uint8_t nodeID = canIDStruct->info.node_id;

		try
		{
			if (canIDStruct->info.direction == 0)
			{
				throw std::runtime_error("Direction bit master to node, ignoring msg...");
			}
			//Don't require mutex at this point, since it is read only after initialization
			bool found = nodeMap.find(nodeID) != nodeMap.end();
			if (!found)
			{
				throw std::runtime_error("Node not found, ignoring msg...");
			}
			Node *node = nodeMap[nodeID];
			if (payloadLength <= 0)
			{
				throw std::runtime_error("CANManager - OnCANRecv: message with 0 payload not supported");
			}
			//TODO: move logic to node
			else if (canMsg->bit.info.channel_id == GENERIC_CHANNEL_ID && canMsg->bit.cmd_id == GENERIC_RES_DATA)
			{
				//TODO: remove when ringbuffer implemented
				node->ProcessSensorDataAndWriteToRingBuffer(canMsg, payloadLength, timestamp);
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
