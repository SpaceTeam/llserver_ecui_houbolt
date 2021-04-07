//
// Created by Markus on 31.03.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_CANDRIVER_H
#define LLSERVER_ECUI_HOUBOLT_CANDRIVER_H

#include "common.h"

#include <vector>
#include <map>
#include <functional>
#include <canlib.h>

class CANDriver
{
    private:
        std::function<void(uint32_t *, uint8_t *, uint32_t *, uint32_t *)> onRecvCallback;
        std::function<void(uint32_t *, uint8_t *, uint32_t *, uint32_t *)> seqRecvCallback;
        std::function<void()> onErrorCallback; //TODO: fix parameters also in constructor

        void OnCANCallback(int handle, void *context, unsigned int event);
        
        canHandle canHandle;
    public:
        CANDriver(unsigned channel, std::function<void(uint32_t *, uint8_t *, uint32_t *, uint32_t *)> onInitRecvCallback,
                std::function<void(uint32_t*, uint8_t *, uint32_t*, uint32_t*)> onRecvCallback,
                std::function<void()> onErrorCallback);
        ~CANDriver();

        //Tells the can driver that initialization is done and canlib callback gets rerouted from initrecvcallback to recvcallback
        void InitDone(void);
        void SendCANMessage(uint32_t canID, uint8_t *payload, uint32_t payloadLength);

        std::map<std::string, bool> GetCANStatusReadable();

};

#endif //LLSERVER_ECUI_HOUBOLT_CANDRIVER_H
