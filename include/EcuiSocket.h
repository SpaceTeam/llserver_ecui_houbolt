//
// Created by Markus on 2019-10-15.
//

#ifndef TXV_ECUI_LLSERVER_ECUISOCKET_H
#define TXV_ECUI_LLSERVER_ECUISOCKET_H

#include <atomic>

#include "common.h"

#include "utility/json.hpp"
#include "utility/Config.h"

#include "driver/Socket.h"

//TODO: turn into singleton
class EcuiSocket
{

private:

    static Socket* socket;

    static std::atomic_bool connectionActive;
    static std::atomic_bool shallClose;

    static std::thread* asyncListenThread;

    static std::function<void()> onCloseCallback;

    EcuiSocket();

    ~EcuiSocket();

    static void Close();

    static void AsyncListen(std::function<void(nlohmann::json)> onMsgCallback);

    static void SendAsync(std::string msg);

public:

    static void Init(std::function<void(nlohmann::json)> onMsgCallback, std::function<void()> onCloseCallback, Config &config);

    static void SendJson(std::string type);
    static void SendJson(std::string type, nlohmann::json content);
    static void SendJson(std::string type, float content);

    static void Destroy();



};


#endif //TXV_ECUI_LLSERVER_ECUISOCKET_H
