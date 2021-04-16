//
// Created by Markus on 31.03.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_NODE_H
#define LLSERVER_ECUI_HOUBOLT_NODE_H

#include <mutex>

#include "common.h"
#include "can/Channel.h"
#include "can/CANDriver.h"
#include "can/CANManager.h"
#include "can_houbolt/channels/generic_channel_def.h"
#include "utility/RingBuffer.h"

class Node : public Channel
{
private:
    //TODO: MP check if this is the only and correct way to implement static const with inheritation
    static const std::vector<std::string> states;
    static const std::map<std::string, std::vector<double>> scalingMap;

private:
    uint8_t canBusChannelID;
	uint8_t nodeID;
	std::map<uint8_t, Channel *> channelMap;
    CANDriver* driver;
    SensorData_t *latestSensorBuffer;
    size_t latestSensorBufferLength;
    std::mutex bufferMtx;

	void InitChannels(NodeInfoMsg_t &nodeInfo, std::map<uint8_t, std::tuple<std::string, double>> &channelInfo);

public:
    //TODO: MP consider if putting channelid as parameter is necessary adapt initializer list if so
	Node(uint8_t nodeID, std::string nodeChannelName, NodeInfoMsg_t &nodeInfo, std::map<uint8_t, std::tuple<std::string, double>> &channelInfo, uint8_t canBusChannelID, CANDriver *driver);
	~Node();

	uint8_t GetNodeID();

    std::vector<std::string> GetStates() override;
	std::map<std::string, std::function<void(std::vector<double> &, bool)>> GetCommands() override;

    void ProcessSensorDataAndWriteToRingBuffer(uint8_t *payload, uint32_t &payloadLength, uint64_t &timestamp, RingBuffer<Sensor_t> &buffer);
    std::map<std::string, std::tuple<double, uint64_t>> GetLatestSensorData();



	void SetBus1Voltage(std::vector<double> &params, bool testOnly=false);
	void GetBus1Voltage(std::vector<double> &params, bool testOnly=false);

	void SetBus2Voltage(std::vector<double> &params, bool testOnly=false);
	void GetBus2Voltage(std::vector<double> &params, bool testOnly=false);

	void SetPowerVoltage(std::vector<double> &params, bool testOnly=false);
	void GetPowerVoltage(std::vector<double> &params, bool testOnly=false);

	void SetPowerCurrent(std::vector<double> &params, bool testOnly=false);
	void GetPowerCurrent(std::vector<double> &params, bool testOnly=false);

	void SetRefreshDivider(std::vector<double> &params, bool testOnly=false);
	void GetRefreshDivider(std::vector<double> &params, bool testOnly=false);

	void SetRefreshRate(std::vector<double> &params, bool testOnly=false);
	void GetRefreshRate(std::vector<double> &params, bool testOnly=false);

};

#endif //LLSERVER_ECUI_HOUBOLT_NODE_H
