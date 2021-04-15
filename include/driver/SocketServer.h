//
// Created by Markus on 15.04.20.
//

#ifndef TXV_ECUI_LLSERVER_SOCKETSERVER_H
#define TXV_ECUI_LLSERVER_SOCKETSERVER_H

#include "common.h"
#include "utility/Config.h"

/**
 *
 * Socket Server accepting only one connection at a time
 */
class SocketServer
{

private:

    int32_t sockfd = -1;
    int32_t connfd = -1;
    int32_t size = std::get<int>(Config::getData("socket_msg_size"));
    uint8_t *buffer;

    std::thread* asyncAcceptThread;

    bool _isConnectionActive = false;
    bool shallClose = false;
    std::function<void()> onCloseCallback;

    std::mutex socketMtx;

    void Open(std::string address, uint16_t port);
    static void AsyncAccept(SocketServer *self);

public:

    SocketServer();

    ~SocketServer();

    void Start(std::string address, uint16_t port, std::function<void()> onCloseCallback);
    void Stop();

    void Send(std::vector<uint8_t> msg);
    std::string Recv();
    std::vector<uint8_t> RecvBytes(uint32_t sizeInBytes);
    bool isConnectionActive();

};


#endif //TXV_ECUI_LLSERVER_SOCKETSERVER_H
