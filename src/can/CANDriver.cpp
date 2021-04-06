//
// Created by Markus on 31.03.21.
//

#include "can/CANDriver.h"

#include <utility>

CANDriver::CANDriver(std::function<void(uint32_t, uint8_t *, uint32_t)> onInitRecvCallback, std::function<void(uint32_t, uint8_t *, uint32_t)> onRecvCallback,
                     std::function<void()> onErrorCallback)
                     : onInitRecvCallback(std::move(onRecvCallback)), onRecvCallback(std::move(onRecvCallback)), onErrorCallback(std::move(onErrorCallback))
{

}

CANDriver::~CANDriver()
{

}

CANResult CANDriver::InitDone()
{
    return CANResult::NOT_IMPLEMENTED;
}

CANResult CANDriver::SendCANMessage(uint32_t canID, uint8_t *payload, uint32_t payloadLength)
{

    return CANResult::NOT_IMPLEMENTED;
}

std::map<std::string, bool> CANDriver::GetCANStatusReadable()
{
    return std::map<std::string, bool>();
}

void CANDriver::OnCANInitRecv()
{
    onInitRecvCallback(0,0,0);
}

void CANDriver::OnCANRecv()
{
    onRecvCallback(0,0,0);
    //TODO: add callbacks and call depending on special_cmd
    //TODO: also call error callback when special_cmd is error
}

void CANDriver::OnCANError()
{
    Debug::error("CAN Error");
    onErrorCallback();
}
