//
// Created by Markus on 2019-09-28.
//

#ifndef TXV_ECUI_LLSERVER_NEWSOCKET_H
#define TXV_ECUI_LLSERVER_NEWSOCKET_H

#include "common.h"
#include "utility/Config.h"

class Socket
{

private:
    int32_t socketfd;
    
    std::string name;
    std::string address;
    uint16_t port;

    bool connectionActive = false;
    bool shallClose = false;
    std::function<void()> onCloseCallback;

    std::mutex socketMtx;

    
public:

    Socket(std::string name, std::function<void()> onCloseCallback, std::string address, uint16_t port);

    ~Socket();

    void Send(const std::string &msg);
    std::string Recv();
    // std::vector<uint8_t> RecvBytes();
    bool isConnectionActive();
    void Close();
    int Connect(int32_t tries=-1);

};


#endif //TXV_ECUI_LLSERVER_NEWSOCKET_H
