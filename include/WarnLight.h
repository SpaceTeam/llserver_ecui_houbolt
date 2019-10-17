//
// Created by Markus on 2019-10-16.
//

#ifndef TXV_ECUI_LLSERVER_WARNLIGHT_H
#define TXV_ECUI_LLSERVER_WARNLIGHT_H

#include "common.h"
#include "Socket.h"

#define WARNLIGHT_PORT 25565

class WarnLight
{

private:

    Socket* socket;

    uint16 id;


public:

    WarnLight(uint16 id);

    ~WarnLight();

    void Error();
    void ServoCal();
    void NoConnection();
    void SafeOn();
    void SafeOff();
    void Standby();

    //RGB(uint8 r, uint8 g, uint8 b);

};


#endif //TXV_ECUI_LLSERVER_WARNLIGHT_H
