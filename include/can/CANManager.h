//
// Created by Markus on 31.03.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_CANMANAGER_H
#define LLSERVER_ECUI_HOUBOLT_CANMANAGER_H

#include "common.h"

#include <map>
#include <functional>
#include <mutex>

#include "Singleton.h"
#include "Node.h"
#include "can/CANMapping.h"
#include "can/CANDriver.h"

//typedef struct
//{
//    nodeId;
//    channelId;
//    name;
//    value;
//} channelData_t;
//
//channelData_t sensorBuffer[];

typedef struct
{
    uint8_t priority;
    uint8_t special_cmd;
    uint8_t node_id;
    uint8_t direction;
} CANID_t;

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

private:
    CANDriver *canDriver;
    std::mutex sensorMtx;
    CANMapping *mapping;

    std::mutex nodeMapMtx;
    std::map<uint8_t, Node *> nodeMap;

    /**
     * key: nodeId << 8 | channelId
     * value: sensorName, scaling
     */
    std::map<uint16_t, std::tuple<std::string, double>> sensorInfoMap;

    //TODO: MP add timestamp to buffer, and maybe to states
	uint32_t *sensorDataBuffer; //this is going to be huge!!!!
	size_t sensorDataBufferLength;

	typedef struct
    {
        uint8_t nodeId;
        uint16_t channelId;
        uint64_t timestamp;
        double data;
    } SensorData_t;

	SensorData_t *latestSensorDataBuffer;
	size_t latestSensorDataBufferLength;

	bool initialized = false;

	~CANManager();

	CANResult RequestCANInfo();
	static inline uint8_t GetNodeID(uint32_t &canID);
	static inline uint16_t MergeNodeIDAndChannelID(uint8_t &nodeId, uint8_t &channelId);
	std::string GetChannelName(uint8_t &nodeID, uint8_t &channelID);

    uint32_t GetNodeCount();
public:

    CANResult Init();

//	std::vector<std::string> GetChannelStates();
//	std::map<std::string, std::function<CANResult(...)>> GetChannelCommands();

	std::map<std::string, std::tuple<double, uint64_t>> GetLatestSensorData();

	void OnChannelStateChanged(std::string stateName, double value, uint64_t timestamp);
	void OnCANInit(uint32_t canID, uint8_t *payload, uint32_t payloadLength, uint64_t timestamp);
	void OnCANRecv(uint32_t canID, uint8_t *payload, uint32_t payloadLength, uint64_t timestamp);

	//TODO: MP add error info to arguments
	void OnCANError();
};

#endif //LLSERVER_ECUI_HOUBOLT_CANMANAGER_H
