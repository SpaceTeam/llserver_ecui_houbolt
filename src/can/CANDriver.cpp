//
// Created by Markus on 31.03.21.
//

#include "can/CANDriver.h"

#include <utility>

CANDriver::CANDriver(std::function<void(uint32_t, uint8_t *, uint32_t)> onRecvCallback,
                     std::function<void()> onErrorCallback) : onRecvCallback(std::move(onRecvCallback)), onErrorCallback(std::move(onErrorCallback))
{

}

CANDriver::~CANDriver()
{

}

CANResult CANDriver::SendCANMessage(uint32_t canID, uint8_t *payload, uint32_t payloadLength)
{
    return CANResult::ERROR;
}

std::map<std::string, bool> CANDriver::GetCANStatusReadable()
{
    return std::map<std::string, bool>();
}

void CANDriver::OnCANRecv()
{
    onRecvCallback(0,{0},0);
}

void CANDriver::OnCANError()
{
    Debug::error("CAN Error");
    onErrorCallback();
}
