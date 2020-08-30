//
// Created by Markus on 2019-09-27.
//

#ifndef TXV_ECUI_LLSERVER_INFLUXINTERFACE_H
#define TXV_ECUI_LLSERVER_INFLUXINTERFACE_H

#include <mutex>
#include <string>

#include "common.h"
#include "Socket.h"

class InfluxInterface {

private:
    static std::mutex socketMtx;

    static Socket* socket;

public:
    static void Init();

    static void Log(std::string msg);

    static void OnClose();

};

#endif //TXV_ECUI_LLSERVER_DEBUG_H
