#pragma once

#ifndef NO_CANLIB

#include "CANDriver.h"
#include <vector>
#include <map>
#include <functional>
#include <string>
#include <canlib.h>
#include <bus_params_tq.h>
#include "common.h"


#define CAN_CHANNELS 3


class CANDriverKvaser : public CANDriver
{
    private:
        canHandle canHandles[CAN_CHANNELS];

		CANParams arbitrationParams;
		CANParams dataParams;

        static void OnCANCallback(int handle, void *driver, unsigned int event);
        std::string CANError(canStatus status);
        canStatus InitializeCANChannel(uint32_t canChannelID);

    public:
        CANDriverKvaser(std::function<void(uint8_t &, uint32_t &, uint8_t *, uint32_t &, uint64_t &)> onRecvCallback,
						std::function<void(std::string *)> onErrorCallback);
        ~CANDriverKvaser();

        void SendCANMessage(uint32_t canBusChannelID, uint32_t canID, uint8_t *payload, uint32_t payloadLength);

        std::map<std::string, bool> GetCANStatusReadable(uint32_t canChannelID);
};

#endif //NO_CANLIB
