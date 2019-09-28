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

    static int32 socketfd;
    static int8 buffer[SOCKET_MSG_SIZE];

    static bool connectionActive;
    static bool shallClose;

    static std::thread* asyncListenThread;

    Socket();

    ~Socket();

    static void asyncListen(std::function<void(int32, json)> onMsgCallback);

public:

    static int init(std::function<void(int32, json)> onMsgCallback);

    static void sendJson(std::string type);
    static void sendJson(std::string type, json content);
    static void sendJson(std::string type, float content);
    static int32 readJson();

    static void destroy();
};


#endif //TXV_ECUI_LLSERVER_SOCKET_H
