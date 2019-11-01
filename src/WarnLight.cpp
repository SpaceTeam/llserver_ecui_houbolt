//
// Created by Markus on 2019-10-16.
//

#include "WarnLight.h"

WarnLight::WarnLight(uint16 id)
{
    this->id = id;
    socket = new Socket(OnClose, WARNLIGHT_IP, WARNLIGHT_PORT, 1);
}

WarnLight::~WarnLight()
{
    delete socket;
}

void WarnLight::Error()
{
    socket->Send("Error\n");
}

void WarnLight::ServoCal()
{
    socket->Send("SerCal\n");
}

void WarnLight::NoConnection()
{
    socket->Send("NoConn\n");
}

void WarnLight::SafeOn()
{
    socket->Send("SafeOn\n");
}

void WarnLight::SafeOff()
{
    socket->Send("SafeOff\n");
}

void WarnLight::Testing()
{
    socket->Send("Testing\n");
}

void WarnLight::Standby()
{
    socket->Send("Standby\n");
}

void WarnLight::OnClose()
{

}
