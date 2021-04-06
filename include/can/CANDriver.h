//
// Created by Markus on 31.03.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_CANDRIVER_H
#define LLSERVER_ECUI_HOUBOLT_CANDRIVER_H

#include "common.h"

#include <vector>
#include <map>
#include <functional>

class CANDriver
{
    private:
        std::function<void(uint32_t, uint8_t *, uint32_t)> onInitRecvCallback; //TODO: fix parameters also in constructor
        std::function<void(uint32_t, uint8_t *, uint32_t)> onRecvCallback; //TODO: fix parameters also in constructor
        std::function<void()> onErrorCallback; //TODO: fix parameters also in constructor

        uint32_t canHandle; //TODO: change to canHandle type

    public:
        //TODO: add filter to canlib so only messages with correct direction bit in id are processed
        CANDriver(std::function<void(uint32_t, uint8_t *, uint32_t)> onInitRecvCallback, std::function<void(uint32_t, uint8_t *, uint32_t)> onRecvCallback, std::function<void()> onErrorCallback);
        ~CANDriver();

        //Tells the can driver that initialization is done and canlib callback gets rerouted from initrecvcallback to recvcallback
        CANResult InitDone();
        CANResult SendCANMessage(uint32_t canID, uint8_t *payload, uint32_t payloadLength);

        std::map<std::string, bool> GetCANStatusReadable();

        void OnCANInitRecv();
        void OnCANRecv(); //TODO: add parameters
        void OnCANError();

};

#endif //LLSERVER_ECUI_HOUBOLT_CANDRIVER_H
