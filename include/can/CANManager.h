#pragma once

#include <atomic>
#include <map>
#include <functional>
#include <mutex>
#include <vector>
#include "common.h"
#include "utility/Singleton.h"
#include "can/Node.h"
#include "can/CANMapping.h"


typedef struct
{
    uint8_t priority;
    uint8_t special_cmd;
    uint8_t node_id;
    uint8_t direction;
} CANID_t;

//typedef struct
//{
//    nodeId;
//    channelId;
//    name;
//    value;
//} channelData_t;
//
//channelData_t sensorBuffer[];


//TODO: MP guarantee that nodeMap is read only after initialization process
/**
 * read node channel mapping on init
 * save scaling
 * read sensor data, convert from byte array to integer
 * then scale sensor data, and add padding to extend to 32 bits
 * write into sensor buffer (its a ring buffer)
 * log same data to database in different thread
 *
 * channel scaling is constant and channel specific
 */
class CANManager : public Singleton<CANManager>
{
    friend class Singleton;
    
	private:


		
		CANDriver *loraDriver;
		CANMapping *mapping;

		std::mutex nodeMapMtx;
		std::map<uint8_t, Node *> nodeMap;

		/**
		 * key: nodeId << 8 | channelId
		 * value: sensorName, scaling
		 */
		std::map<uint16_t, std::tuple<std::string, std::vector<double>>> sensorInfoMap;

		std::atomic_bool initialized = false;
		std::atomic_bool useLora = false;

		CANResult RequestCANInfo(CANDriver *driver, std::vector<uint32_t> &canBusChannelIDs);
		static inline uint8_t GetNodeID(uint32_t &canID);
		static inline uint16_t MergeNodeIDAndChannelID(uint8_t &nodeId, uint8_t &channelId);

		void InitializeNode(uint8_t canBusChannelID, uint8_t nodeID, NodeInfoMsg_t *nodeInfo, CANDriver *driver);

		void RequestCurrentState();

		~CANManager();
	public:
		CANDriver *canDriver;

		CANResult Init();

		bool IsInitialized();

//		std::vector<std::string> GetChannelStates();
//		std::map<std::string, std::function<CANResult(...)>> GetChannelCommands();

		std::map<std::string, std::tuple<double, uint64_t>> GetLatestSensorData();

		void OnChannelStateChanged(std::string stateName, double value, uint64_t timestamp);
		void OnCANRecv(uint8_t canBusChannelID, uint32_t canID, uint8_t *payload, uint32_t payloadLength, uint64_t timestamp, CANDriver *canDriver);

		//TODO: MP add error info to arguments
		void OnCANError(std::string *error);

		//-------------------------------Utility Functions-------------------------------//

		void ResetOffset(std::vector<double> &params, bool testOnly);
};
