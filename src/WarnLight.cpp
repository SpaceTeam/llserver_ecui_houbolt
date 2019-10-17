//
// Created by Markus on 2019-10-16.
//

#include "WarnLight.h"

WarnLight::WarnLight(uint16 id)
{
    this->id = id;
    socket = new Socket(WARNLIGHT_PORT);
}

WarnLight::~WarnLight()
{
    delete socket;
}

void WarnLight::Error()
{
    socket->Send("Error");
}

void WarnLight::ServoCal()
{
    socket->Send("SerCal");
}

void WarnLight::NoConnection()
{
    socket->Send("NoConn");
}

void WarnLight::SafeOn()
{
    socket->Send("SafeOn");
}

void WarnLight::SafeOff()
{
    socket->Send("SafeOff");
}

void WarnLight::Standby()
{
    socket->Send("Standby");
}