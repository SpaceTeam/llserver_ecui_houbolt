//
// Created by Markus on 31.03.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_CANMANAGER_H
#define LLSERVER_ECUI_HOUBOLT_CANMANAGER_H

#include "common.h"

#include <map>
#include <functional>
#include <mutex>

#include "utility/Singleton.h"
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
    double value;
    uint64_t timestamp;
} SensorData_t;

typedef struct
{
    union
    {
        struct
        {
            uint8_t nodeId;
            uint8_t channelId;
        } separate;
        uint16_t nodeChannelID;
    };
    SensorData_t data;
} Sensor_t;

typedef struct
{
    uint8_t priority;
    uint8_t special_cmd;
    uint8_t node_id;
    uint8_t direction;
} CANID_t;

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

private:
    CANDriver *canDriver;
    CANMapping *mapping;

    std::mutex nodeMapMtx;
    std::map<uint8_t, Node *> nodeMap;

    /**
     * key: nodeId << 8 | channelId
     * value: sensorName, scaling
     */
    std::map<uint16_t, std::tuple<std::string, double>> sensorInfoMap;

	bool initialized = false;

	CANResult RequestCANInfo();
	static inline uint8_t GetNodeID(uint32_t &canID);
	static inline uint16_t MergeNodeIDAndChannelID(uint8_t &nodeId, uint8_t &channelId);

public:
    ~CANManager();

    CANResult Init();

//	std::vector<std::string> GetChannelStates();
//	std::map<std::string, std::function<CANResult(...)>> GetChannelCommands();

	std::map<std::string, std::tuple<double, uint64_t>> GetLatestSensorData();

	void OnChannelStateChanged(std::string stateName, double value, uint64_t timestamp);
	void OnCANInit(uint8_t canBusChannelID, uint32_t canID, uint8_t *payload, uint32_t payloadLength, uint64_t timestamp);
	void OnCANRecv(uint8_t canBusChannelID, uint32_t canID, uint8_t *payload, uint32_t payloadLength, uint64_t timestamp);

	//TODO: MP add error info to arguments
	void OnCANError(std::string *error);


};

#endif //LLSERVER_ECUI_HOUBOLT_CANMANAGER_H
