//
// Created by Markus on 2019-10-15.
//

#ifndef TXV_ECUI_LLSERVER_ECUISOCKET_H
#define TXV_ECUI_LLSERVER_ECUISOCKET_H

#include "Socket.h"

class EcuiSocket
{

private:

    static Socket* socket;

    static bool connectionActive;
    static bool shallClose;

    static std::thread* asyncListenThread;

    EcuiSocket();

    ~EcuiSocket();

    static void AsyncListen(std::function<void(nlohmann::json)> onMsgCallback);

public:

    static void Init(std::function<void(nlohmann::json)> onMsgCallback);

    static void SendJson(std::string type);
    static void SendJson(std::string type, nlohmann::json content);
    static void SendJson(std::string type, float content);

    static void Destroy();



};


#endif //TXV_ECUI_LLSERVER_ECUISOCKET_H
