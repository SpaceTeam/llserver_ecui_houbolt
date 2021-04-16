//
// Created by Markus on 05.12.19.
//

#ifndef TXV_ECUI_LLSERVER_TMPOE_H
#define TXV_ECUI_LLSERVER_TMPOE_H

#include "common.h"
#include "driver/SocketOld.h"

class TMPoE
{

private:
    SocketOld* socket;

    std::mutex readMtx;

    uint16_t id;

    std::vector<uint32_t> currValues;

    std::thread* asyncListenThread;

    bool shallClose = false;

    static void OnClose();
    static void AsyncListen(TMPoE *self, uint32_t sampleRate);
public:

    TMPoE(uint16_t id, uint32_t sampleRate);

    ~TMPoE();

    std::vector<uint32_t> Read();


};


#endif //TXV_ECUI_LLSERVER_TMPOE_H
