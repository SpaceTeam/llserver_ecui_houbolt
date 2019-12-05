//
// Created by Markus on 05.12.19.
//

#ifndef TXV_ECUI_LLSERVER_TMPOE_H
#define TXV_ECUI_LLSERVER_TMPOE_H

#include "common.h"
#include "Socket.h"

#define TMPoE_PORT 25557

class TMPoE
{

private:
    Socket* socket;

    std::mutex readMtx;

    uint16 id;

    std::vector<uint32> currValues;

    std::thread* asyncListenThread;

    bool shallClose = false;

    static void OnClose();
    static void AsyncListen(TMPoE *self, uint32 sweepFrequency);
public:

    TMPoE(uint16 id, uint32 sweepFrequency);

    ~TMPoE();

    std::vector<uint32> Read();


};


#endif //TXV_ECUI_LLSERVER_TMPOE_H
