//
// Created by Markus on 2019-10-16.
//

#ifndef TXV_ECUI_LLSERVER_WARNLIGHT_H
#define TXV_ECUI_LLSERVER_WARNLIGHT_H

#include "common.h"
#include "drivers/SocketOld.h"
#include "json.hpp"

class WarnLight
{

private:

    SocketOld* socket;

    uint16_t id;

    static void OnClose();
    void SendJson(nlohmann::json message);
public:

    WarnLight(uint16_t id);

    ~WarnLight();

    void Reset();
    void SetColor(uint8_t red, uint8_t green, uint8_t blue);
    void SetMode(std::string mode);
    void StopBuzzer();
    void StartBuzzerBeep(uint16_t time);
    void StartBuzzerContinuous();
};


#endif //TXV_ECUI_LLSERVER_WARNLIGHT_H
