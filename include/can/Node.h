//
// Created by Markus on 31.03.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_NODE_H
#define LLSERVER_ECUI_HOUBOLT_NODE_H

#include <mutex>
#include <atomic>

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
	uint8_t channel_data[60];
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
#include "logging/InfluxDbLogger.h"

class Node : public Channel
{
private:
    //TODO: MP check if this is the only and correct way to implement static const with inheritation
    static const std::vector<std::string> states;
    static const std::map<std::string, std::vector<double>> scalingMap;
    static const std::map<GENERIC_VARIABLES, std::string> variableMap;
    static InfluxDbLogger *logger;
    static std::mutex loggerMtx;

private:
    uint8_t canBusChannelID;
	uint8_t nodeID;
    uint32_t firwareVersion;
	std::map<uint8_t, Channel *> channelMap;
    CANDriver* driver;
    SensorData_t *latestSensorBuffer;
    size_t latestSensorBufferLength = 0;
    std::mutex bufferMtx;

	void InitChannels(NodeInfoMsg_t &nodeInfo, std::map<uint8_t, std::tuple<std::string, std::vector<double>>> &channelInfo);

	//-------------------------------RECEIVE Functions-------------------------------//

    void NodeStatusResponse(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp);
    void ResetAllSettingsResponse(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp);

public:
    std::atomic_uint64_t count = 0;

    //TODO: MP consider if putting channelid as parameter is necessary adapt initializer list if so
	Node(uint8_t nodeID, std::string nodeChannelName, NodeInfoMsg_t &nodeInfo, std::map<uint8_t, std::tuple<std::string, std::vector<double>>> &channelInfo, uint8_t canBusChannelID, CANDriver *driver);
	~Node();

	//-------------------------------GETTER & SETTER Functions-------------------------------//

	uint8_t GetNodeID();
    uint32_t GetFirmwareVersion();
	CANDriver *GetCANDriver();
	uint8_t GetCANBusChannelID();

    std::vector<std::string> GetStates() override;
	std::map<std::string, command_t> GetCommands() override;
    std::map<std::string, std::tuple<double, uint64_t>> GetLatestSensorData();

    //-------------------------------RECEIVE Functions-------------------------------//

    void ProcessSensorDataAndWriteToRingBuffer(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp, RingBuffer<Sensor_t> &buffer);

    void ProcessCANCommand(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp) override;

    //-------------------------------SEND Functions-------------------------------//

	void SetBus1Voltage(std::vector<double> &params, bool testOnly);
	void GetBus1Voltage(std::vector<double> &params, bool testOnly);

	void SetBus2Voltage(std::vector<double> &params, bool testOnly);
	void GetBus2Voltage(std::vector<double> &params, bool testOnly);

	void SetPowerVoltage(std::vector<double> &params, bool testOnly);
	void GetPowerVoltage(std::vector<double> &params, bool testOnly);

	void SetPowerCurrent(std::vector<double> &params, bool testOnly);
	void GetPowerCurrent(std::vector<double> &params, bool testOnly);

	void SetRefreshDivider(std::vector<double> &params, bool testOnly);
	void GetRefreshDivider(std::vector<double> &params, bool testOnly);

	void SetRefreshRate(std::vector<double> &params, bool testOnly);
	void GetRefreshRate(std::vector<double> &params, bool testOnly);

	void SetUARTEnabled(std::vector<double> &params, bool testOnly);
	void GetUARTEnabled(std::vector<double> &params, bool testOnly);

	void RequestSetSpeaker(std::vector<double> &params, bool testOnly);

	void RequestData(std::vector<double> &params, bool testOnly);
	void RequestNodeStatus(std::vector<double> &params, bool testOnly);
	void RequestResetAllSettings(std::vector<double> &params, bool testOnly);
};

class NonNodeChannel
{
protected:
    Node *parent;

public:
    NonNodeChannel(Node *parent) : parent(parent) {};
};

#endif //LLSERVER_ECUI_HOUBOLT_NODE_H
