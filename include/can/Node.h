//
// Created by Markus on 31.03.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_NODE_H
#define LLSERVER_ECUI_HOUBOLT_NODE_H

#include <mutex>

#include "common.h"

typedef struct
{
    double value;
    uint64_t timestamp;
} SensorData_t;

//TODO: move to can_houbolt headers
typedef struct __attribute__((__packed__))
{
	uint32_t channel_mask;
	uint8_t *channel_data;
} SensorMsg_t;

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

#include "can/Channel.h"
#include "can/CANDriver.h"
#include "can_houbolt/channels/generic_channel_def.h"
#include "utility/RingBuffer.h"

class Node : public Channel
{
private:
    //TODO: MP check if this is the only and correct way to implement static const with inheritation
    static const std::vector<std::string> states;
    static const std::map<std::string, std::vector<double>> scalingMap;
    static const std::map<GENERIC_VARIABLES, std::string> variableMap;

private:
    uint8_t canBusChannelID;
	uint8_t nodeID;
	std::map<uint8_t, Channel *> channelMap;
    CANDriver* driver;
    SensorData_t *latestSensorBuffer;
    size_t latestSensorBufferLength;
    std::mutex bufferMtx;

	void InitChannels(NodeInfoMsg_t &nodeInfo, std::map<uint8_t, std::tuple<std::string, double>> &channelInfo);

	//-------------------------------RECEIVE Functions-------------------------------//

    void NodeStatusResponse(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp);
    void ResetAllSettingsResponse(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp);

public:
    //TODO: MP consider if putting channelid as parameter is necessary adapt initializer list if so
	Node(uint8_t nodeID, std::string nodeChannelName, NodeInfoMsg_t &nodeInfo, std::map<uint8_t, std::tuple<std::string, double>> &channelInfo, uint8_t canBusChannelID, CANDriver *driver);
	~Node();

	//-------------------------------GETTER & SETTER Functions-------------------------------//

	uint8_t GetNodeID();
	CANDriver *GetCANDriver();
	uint8_t GetCANBusChannelID();

    std::vector<std::string> GetStates() override;
	std::map<std::string, std::function<void(std::vector<double> &, bool)>> GetCommands() override;
    std::map<std::string, std::tuple<double, uint64_t>> GetLatestSensorData();

    //-------------------------------RECEIVE Functions-------------------------------//

    void ProcessSensorDataAndWriteToRingBuffer(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp, RingBuffer<Sensor_t> &buffer);

    void ProcessCANCommand(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp) override;

    //-------------------------------SEND Functions-------------------------------//

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

	void SetUARTEnabled(std::vector<double> &params, bool testOnly=false);
	void GetUARTEnabled(std::vector<double> &params, bool testOnly=false);

	void RequestSetSpeaker(std::vector<double> &params, bool testOnly=false);

	void RequestData(std::vector<double> &params, bool testOnly=false);
	void RequestNodeStatus(std::vector<double> &params, bool testOnly=false);
	void RequestResetAllSettings(std::vector<double> &params, bool testOnly=false);
};

class NonNodeChannel
{
protected:
    Node *parent;

public:
    NonNodeChannel(Node *parent) : parent(parent) {};
};

#endif //LLSERVER_ECUI_HOUBOLT_NODE_H
