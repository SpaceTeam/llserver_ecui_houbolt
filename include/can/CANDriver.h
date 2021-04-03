//
// Created by Markus on 31.03.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_CANDRIVER_H
#define LLSERVER_ECUI_HOUBOLT_CANDRIVER_H

#include "common.h"

#include <functional>

class CANDriver
{
    private:
        std::function<void(uint32_t, uint8_t*, uint32_t)> onRecvCallback; //TODO: fix parameters also in constructor

    public:
        CANDriver(std::function<void(uint32_t, uint8_t*, uint32_t)> onRecvCallback) : onRecvCallback(onRecvCallback);
        ~CANDriver();

        CANResult SendCANMessage(uint32_t canID, uint8_t* payload, uint32_t payloadLength);

        void OnCANRecv(); //TODO: add parameters

};

#endif //LLSERVER_ECUI_HOUBOLT_CANDRIVER_H
