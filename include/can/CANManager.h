//
// Created by Markus on 31.03.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_CANMANAGER_H
#define LLSERVER_ECUI_HOUBOLT_CANMANAGER_H

#include "common.h"

#include <map>
#include <functional>

#include "Node.h"
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
class CANManager
{

private:
    static CANManager* instance;
    CANDriver* canDriver;

    std::map<uint8_t, Node> nodes;

	uint32_t* sensorBuffer; //this is going to be huge!!!!
	uint32_t* latestSensorDataBuffer;

	bool initialized = false;

	CANManager(const CANManager& copy);
	CANManager();
	~CANManager();
public:
    static CANManager* Instance();

    CANResult Init();

	std::vector<std::string> GetChannelStates();
	std::map<std::string, std::function<CANResult(...)>> GetChannelCommands();
	std::map<std::string, double> GetLatestSensorData();

	void OnChannelStateChanged(std::string, double);
	void OnCANRecv(uint32_t canID, uint8_t* payload, uint32_t payloadLength);
	void OnCANError();
};

#endif //LLSERVER_ECUI_HOUBOLT_CANMANAGER_H
