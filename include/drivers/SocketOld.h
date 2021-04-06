//
// Created by Markus on 2019-09-28.
//

#ifndef TXV_ECUI_LLSERVER_SOCKET_H
#define TXV_ECUI_LLSERVER_SOCKET_H

#include "common.h"
#include "Config.h"

class SocketOld
{

private:

    int32_t socketfd;
    int size = std::get<int>(Config::getData("socket_msg_size"));
    uint8_t *buffer;

    bool connectionActive = false;
    bool shallClose = false;
    std::function<void()> onCloseCallback;

    std::mutex socketMtx;

    void Connect(std::string address, uint16_t port, int32_t tries=-1);
    void Close();
public:

    SocketOld(std::function<void()> onCloseCallback, std::string address, uint16_t port, int32_t tries=-1);

    ~SocketOld();

    void Send(std::string msg);
    std::string Recv();
    std::string newRecv();
    std::vector<uint8_t> RecvBytes();
    bool isConnectionActive();

};


#endif //TXV_ECUI_LLSERVER_SOCKET_H
