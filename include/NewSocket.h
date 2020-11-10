//
// Created by Markus on 2019-09-28.
//

#ifndef TXV_ECUI_LLSERVER_NEWSOCKET_H
#define TXV_ECUI_LLSERVER_NEWSOCKET_H

#include "common.h"
#include "Config.h"

class NewSocket
{

private:
    int32 socketfd;
    
    std::string name;
    std::string address;
    uint16 port;

    bool connectionActive = false;
    bool shallClose = false;
    std::function<void()> onCloseCallback;

    std::mutex socketMtx;

    
public:

    NewSocket(std::string name, std::function<void()> onCloseCallback, std::string address, uint16 port);

    ~NewSocket();

    void Send(std::string msg);
    std::string Recv();
    // std::vector<uint8> RecvBytes();
    bool isConnectionActive();
    void Close();
    int Connect(int32 tries=-1);

};


#endif //TXV_ECUI_LLSERVER_NEWSOCKET_H
