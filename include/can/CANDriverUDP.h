#pragma once

#include <vector>
#include <map>
#include <functional>
#include <string>
#include <thread>
#include <atomic>
#include "common.h"
#include "utility/Config.h"
#include "CANDriver.h"
#include "driver/UDPSocket.h"

class CANDriverUDP : public CANDriver
{
private:
    typedef enum class MessageType_t
    {
        RESERVED = 0,
        DATAFRAME = 1,
        HEARTBEAT = 2
    } MessageType;

    typedef enum class CanMessageOption_t
    {
        EMPTY = 0,
        USED = 1
    } CanMessageOption;

    typedef struct
    {
        uint8_t msgType;
        uint64_t timestamp;
        uint8_t *payload;
    } CANDriverUDPMessage;

    UDPSocket *socket;

    std::atomic_bool connectionActive;
    std::atomic_bool shallClose;

    std::vector<uint32_t> nodeIDs;
    std::vector<uint32_t> canMsgSizes;
    uint32_t totalRequiredMsgPayloadSize = 0;
    const uint32_t MSG_HEADER_SIZE = 0;
    const uint32_t CAN_MSG_HEADER_SIZE = 1;

    std::thread *asyncListenThread;

    void Close();
    void AsyncListen();

public:
    CANDriverUDP(std::function<void(uint8_t &, uint32_t &, uint8_t *, uint32_t &, uint64_t &, CANDriver *driver)> onRecvCallback,
                 std::function<void(std::string *)> onErrorCallback, Config &config);
    ~CANDriverUDP();

    void SendCANMessage(uint32_t canBusChannelID, uint32_t canID, uint8_t *payload, uint32_t payloadLength, bool blocking);

    std::map<std::string, bool> GetCANStatusReadable(uint32_t canChannelID);
};
