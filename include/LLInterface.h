//
// Created by Markus on 2019-10-15.
//

#ifndef TXV_ECUI_LLSERVER_LLINTERFACE_H
#define TXV_ECUI_LLSERVER_LLINTERFACE_H

#include "common.h"
#include "I2C.h"

#include "json.hpp"

#include "WarnLight.h"

class LLInterface
{

private:

    static I2C* i2cDevice;
    static WarnLight* warnLight;
    //static GPIO[] gpioDevices;

    //static SPI* spiDevice;

    LLInterface();

    ~LLInterface();
public:

    static void Init();
    static void Destroy();

    static void EnableAllOutputDevices();
    static void DisableAllOutputDevices();

    static std::vector<std::string> GetAllSensorNames();
    static std::map<std::string, int32> GetAllSensors();

    static nlohmann::json GetAllServoData();

    static void turnRed();
    static void turnGreen();


    static bool ExecCommand(std::string name, json value);

};


#endif //TXV_ECUI_LLSERVER_LLINTERFACE_H
