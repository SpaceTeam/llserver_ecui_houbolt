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
    uint8 buffer[SOCKET_MSG_SIZE];

    bool connectionActive = false;
    bool shallClose = false;
    std::function<void()> onCloseCallback;

    std::mutex socketMtx;

    void Connect(std::string address, uint16 port, int32 tries=-1);
    void Close();
public:

    Socket(std::function<void()> onCloseCallback, std::string address, uint16 port, int32 tries=-1);

    ~Socket();

    void Send(std::string msg);
    std::string Recv();
    std::vector<uint8> RecvBytes();
    bool isConnectionActive();

};


#endif //TXV_ECUI_LLSERVER_SOCKET_H
