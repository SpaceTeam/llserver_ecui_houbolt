//
// Created by Markus on 2019-10-16.
//

#ifndef TXV_ECUI_LLSERVER_WARNLIGHT_H
#define TXV_ECUI_LLSERVER_WARNLIGHT_H

#include "common.h"
#include "Socket.h"
#include "json.txt"

class WarnLight
{

private:

    Socket* socket;

    uint16 id;

    static void OnClose();
    void SendJson(json message);
public:

    WarnLight(uint16 id);

    ~WarnLight();

    void Reset();
    void SetColor(uint8 red, uint8 green, uint8 blue);
    void SetMode(std::string mode);
    void StopBuzzer();
    void StartBuzzerBeep(uint16 time);
    void StartBuzzerContinuous();
};


#endif //TXV_ECUI_LLSERVER_WARNLIGHT_H
