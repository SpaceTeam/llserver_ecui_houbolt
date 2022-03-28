#include "can/CANDriver.h"
#include <iostream>


CANDriver::CANDriver(std::function<void(uint8_t &, uint32_t &, uint8_t *, uint32_t &, uint64_t &)> onRecvCallback,
                     std::function<void(std::string *)> onErrorCallback) :
	onRecvCallback(std::move(onRecvCallback)),
	onErrorCallback(std::move(onErrorCallback))
{

}

CANDriver::~CANDriver()
{

}

void CANDriver::SendCANMessage(uint32_t canBusChannelID, uint32_t canID, uint8_t *payload, uint32_t payloadLength)
{
	std::cerr << "CANDriver::SendCANMessage called, probably in error" << std::endl;
}

std::map<std::string, bool> CANDriver::GetCANStatusReadable(uint32_t canChannelID)
{
	std::cerr << "CANDriver::GetCANStatusReadable called, probably in error" << std::endl;
}
