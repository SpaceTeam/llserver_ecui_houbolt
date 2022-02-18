#pragma once

#include <vector>
#include <map>
#include <functional>
#include <string>
#include "common.h"
#include "CANDriver.h"


class CANDriverSocketCAN : public CANDriver
{
    private:
//        canHandle canHandles[CAN_CHANNELS];
//
//        static void OnCANCallback(int handle, void *driver, unsigned int event);
//        std::string CANError(canStatus status);
//        canStatus InitializeCANChannel(uint32_t canChannelID);

    public:
        CANDriverSocketCAN(std::function<void(uint8_t &, uint32_t &, uint8_t *, uint32_t &, uint64_t &)> onRecvCallback,
					   	   std::function<void(std::string *)> onErrorCallback,
					   	   CANParams arbitrationParams,
					   	   CANParams dataParams);
        ~CANDriverSocketCAN();

        //Tells the can driver that initialization is done and canlib callback gets rerouted from initrecvcallback to recvcallback
        void InitDone(void);
        void SendCANMessage(uint32_t canBusChannelID, uint32_t canID, uint8_t *payload, uint32_t payloadLength);

        std::map<std::string, bool> GetCANStatusReadable(uint32_t canChannelID);
};
