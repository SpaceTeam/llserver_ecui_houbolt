//
// Created by Markus on 2019-09-27.
//

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <mutex>
#include <thread>
#include <stdlib.h>
#include <signal.h>

#include "InfluxInterface.h"

#include "Config.h"

std::mutex InfluxInterface::socketMtx;
Socket* InfluxInterface::socket;

void InfluxInterface::Init()
{
    std::string ip = std::get<std::string>(Config::getData("LOGGING/ip"));
    int32 port = std::get<int>(Config::getData("LOGGING/port"));
    socket = new Socket(OnClose, ip, port, -1);
}

void InfluxInterface::OnClose()
{
    Debug::info("influx interface socket closed");
}

void InfluxInterface::Log(std::string msg)
{
    std::lock_guard<std::mutex> lock(socketMtx);
    socket->Send(msg);
}
