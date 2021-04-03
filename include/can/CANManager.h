//
// Created by Markus on 31.03.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_CANMANAGER_H
#define LLSERVER_ECUI_HOUBOLT_CANMANAGER_H

#include "common.h"

#include <map>
#include <functional>

#include "Channel.h"

typedef struct
{
    nodeId;
    channelId;
    name;
    value;
} channelData_t;

channelData_t sensorBuffer[];

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
	static std::map<uint32_t, Channel> channels;

	static uint32_t sensorBuffer[]; //this is going to be huge!!!!
	static uint32_t latestSensorDataBuffer[];

	CANManager();
	~CANManager();
public:
	static CANResult Init();

	static std::vector<std::string> GetChannelStates();
	static std::map<std::string, std::function<CANResult(...)>> GetChannelCommands();
	static std::map<std::string, double> GetLatestSensorData();

	static void OnChannelStateChanged(std::string, double);

};

#endif //LLSERVER_ECUI_HOUBOLT_CANMANAGER_H
