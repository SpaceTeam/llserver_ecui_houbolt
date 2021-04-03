//
// Created by Markus on 2019-10-16.
//

#include "drivers/WarnLight.h"
#include "Config.h"

using json = nlohmann::json;

WarnLight::WarnLight(uint16 id)
{
    this->id = id;
    std::string ip = std::get<std::string>(Config::getData("WARNLIGHT/ip"));
    int32 port = std::get<int>(Config::getData("WARNLIGHT/port"));
    socket = new SocketOld(OnClose, ip, port, 1);
    Reset();
}

WarnLight::~WarnLight()
{
    delete socket;
}

void WarnLight::SendJson(json message)
{
    socket->Send(message.dump() + "\n");
}

void WarnLight::Reset()
{
    json message;
    message["type"] = "reset";
    SendJson(message);
}

void WarnLight::SetColor(uint8 red, uint8 green, uint8 blue)
{
    json message = {
        {"type", "set-color"},
        {"content", {
            {"red", red},
            {"green", green},
            {"blue", blue},
        }}
    };
    SendJson(message);
}

void WarnLight::SetMode(std::string mode) {
    json message = {
        {"type", "set-mode"},
        {"content", {
            {"mode", mode}
        }}
    };
    SendJson(message);
}

void WarnLight::StopBuzzer()
{
    json message;
    message["type"] = "stop-buzzer";
    SendJson(message);
}

void WarnLight::StartBuzzerBeep(uint16 time)
{
    json message = {
        {"type", "start-buzzer-beep"},
        {"content", {
            {"period", time}
        }}
    };
    SendJson(message);
}

void WarnLight::StartBuzzerContinuous()
{
    json message;
    message["type"] = "start-buzzer-continuous";
    SendJson(message);
}

void WarnLight::OnClose()
{

}
