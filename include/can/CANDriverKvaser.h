#pragma once

#ifndef NO_CANLIB

#include "CANDriver.h"


#include <vector>
#include <map>
#include <functional>
#include <string>
#include <canlib.h>
#include "common.h"

class CANDriverKvaser : public CANDriver
{
    private:
        std::map<uint32_t, canHandle> canHandlesMap = std::map<uint32_t, canHandle>();

		CANParams arbitrationParams;
		CANParams dataParams;

        uint64_t blockingTimeout;

        static void OnCANCallback(int handle, void *driver, unsigned int event);
        std::string CANError(canStatus status);
        canStatus InitializeCANChannel(uint32_t canChannelID);

    public:
        CANDriverKvaser(std::function<void(uint8_t &, uint32_t &, uint8_t *, uint32_t &, uint64_t &, CANDriver *driver)> onRecvCallback,
						std::function<void(std::string *)> onErrorCallback, std::vector<uint32_t> &canBusChannelIDs);
        ~CANDriverKvaser();

        void SendCANMessage(uint32_t canBusChannelID, uint32_t canID, uint8_t *payload, uint32_t payloadLength, bool blocking);

        std::map<std::string, bool> GetCANStatusReadable(uint32_t canChannelID);
};

#endif //NO_CANLIB
