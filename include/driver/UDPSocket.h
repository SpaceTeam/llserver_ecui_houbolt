//
// Created by Markus on 2019-09-28.
//

#ifndef TXV_ECUI_LLSERVER_NEWSOCKET_H
#define TXV_ECUI_LLSERVER_NEWSOCKET_H

#include <arpa/inet.h>

#include "common.h"
#include "utility/Config.h"

typedef struct
{
    size_t dataLength; 
    uint8_t * data;
} UDPMessage;

class UDPSocket
{

private:
    int32_t socketfd;
    
    std::string name;
    std::string recvAddress;
    uint16_t recvPort;
    std::string address;
    uint16_t port;

    struct sockaddr_in serv_addr;
    socklen_t serv_addr_len;

    struct sockaddr_in recv_addr;
    socklen_t recv_addr_len;

    uint8_t *buffer;

    bool connectionActive = false;
    bool shallClose = false;
    std::function<void()> onCloseCallback;

    std::mutex socketMtx;
    
public:

    UDPSocket(std::string name, std::function<void()> onCloseCallback, std::string address, uint16_t port);

    ~UDPSocket();

    void Send(UDPMessage *msg);
    void Recv(UDPMessage *msg);
    // std::vector<uint8_t> RecvBytes();
    bool isConnectionActive();
    void Close();
    int Connect(int32_t tries=-1);

};


#endif //TXV_ECUI_LLSERVER_NEWSOCKET_H
