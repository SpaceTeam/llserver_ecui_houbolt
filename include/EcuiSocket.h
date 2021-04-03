//
// Created by Markus on 2019-10-15.
//

#ifndef TXV_ECUI_LLSERVER_ECUISOCKET_H
#define TXV_ECUI_LLSERVER_ECUISOCKET_H

#include "drivers/Socket.h"

class EcuiSocket
{

private:

    static Socket* socket;

    static bool connectionActive;
    static bool shallClose;

    static std::thread* asyncListenThread;

    static std::function<void()> onCloseCallback;

    EcuiSocket();

    ~EcuiSocket();

    static void Close();

    static void AsyncListen(std::function<void(nlohmann::json)> onMsgCallback);

    static void SendAsync(std::string msg);

public:

    static void Init(std::function<void(nlohmann::json)> onMsgCallback, std::function<void()> onCloseCallback);

    static void SendJson(std::string type);
    static void SendJson(std::string type, nlohmann::json content);
    static void SendJson(std::string type, float content);

    static void Destroy();



};


#endif //TXV_ECUI_LLSERVER_ECUISOCKET_H
