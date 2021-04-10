//
// Created by Markus on 31.03.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_CANDRIVER_H
#define LLSERVER_ECUI_HOUBOLT_CANDRIVER_H

#include "common.h"

#include <vector>
#include <map>
#include <functional>
#include <string>
#include <canlib.h>
#include <bus_params_tq.h>

#define CAN_CHANNELS 4

class CANDriver
{
    private:
        std::function<void(uint8_t &, uint32_t &, uint8_t *, uint32_t &, uint64_t &)> onRecvCallback;
        std::function<void(uint8_t &, uint32_t &, uint8_t *, uint32_t &, uint64_t &)> seqRecvCallback;
        std::function<void(std::string *)> onErrorCallback;

        kvBusParamsTq arbitrationParams;
        kvBusParamsTq dataParams;
        canHandle canHandles[CAN_CHANNELS];

        static void OnCANCallback(int handle, void *driver, unsigned int event);
        std::string CANError(canStatus status);
        canStatus InitializeCANChannel(uint32_t canChannelID);

    public:
        CANDriver(std::function<void(uint8_t &, uint32_t &, uint8_t *, uint32_t &, uint64_t &)> onInitRecvCallback,
                  std::function<void(uint8_t &, uint32_t &, uint8_t *, uint32_t &, uint64_t &)> onRecvCallback,
                  std::function<void(std::string *)> onErrorCallback, kvBusParamsTq arbitrationParams, kvBusParamsTq dataParams);
        ~CANDriver();

        //Tells the can driver that initialization is done and canlib callback gets rerouted from initrecvcallback to recvcallback
        void InitDone(void);
        void SendCANMessage(uint32_t canBusChannelID, uint32_t canID, uint8_t *payload, uint32_t payloadLength);

        std::map<std::string, bool> GetCANStatusReadable(uint32_t canChannelID);

};

#endif //LLSERVER_ECUI_HOUBOLT_CANDRIVER_H
