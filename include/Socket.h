//
// Created by Markus on 2019-09-28.
//

#ifndef TXV_ECUI_LLSERVER_SOCKET_H
#define TXV_ECUI_LLSERVER_SOCKET_H

#include "common.h"
#include "config.h"

class Socket
{

private:

    int32 socketfd;
    int8 buffer[SOCKET_MSG_SIZE];

    bool connectionActive;
    bool shallClose;

    std::mutex socketMtx;

    void Connect(uint16 port);
public:

    Socket(uint16 port);

    ~Socket();

    void Send(std::string msg);
    std::string Recv();
};


#endif //TXV_ECUI_LLSERVER_SOCKET_H
