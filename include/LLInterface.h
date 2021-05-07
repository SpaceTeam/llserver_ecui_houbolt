//
// Created by Markus on 2019-10-15.
//

#ifndef TXV_ECUI_LLSERVER_LLINTERFACE_H
#define TXV_ECUI_LLSERVER_LLINTERFACE_H

#include "common.h"
#include "I2C.h"

#include "json.hpp"

#include "WarnLight.h"
#include "TmPoE.h"

enum class WarningLightStatus
{
    SAFE,
    RESTRICTED,
    CRITICAL
};

class LLInterface
{

private:

    static I2C* i2cDevice;
    static WarnLight* warnLight;
    static TMPoE *tmPoE;
    //static GPIO[] gpioDevices;

    //static SPI* spiDevice;

    static bool isInitialized;

    static bool useTMPoE;

    static bool isTransmittingSensors;
    static WarningLightStatus warningLightStatus;
    static Timer* sensorTimer;

    static void GetSensors(int64 microTime);
    static void StopGetSensors();

    LLInterface();

    ~LLInterface();
public:

    static void Init();
    static void Destroy();

    static void EnableAllOutputDevices();
    static void DisableAllOutputDevices();

    static void TransmitSensors(int64 microTime, std::map<std::string, double> sensors);

    static std::vector<std::string> GetAllSensorNames();
    static std::map<std::string, double> GetAllSensors();

    static std::vector<std::string> GetAllOutputNames();

    static void StartSensorTransmission();
    static void StopSensorTransmission();

    static nlohmann::json GetAllServoData();

    static void TurnRed();
    static void TurnGreen();
    static void TurnYellow();
    static void BeepRed();
	static int32 GetWarningLightStatus();
    static void SetWarningLightStatus(WarningLightStatus status, bool sendState=true);

    static bool ExecCommand(std::string name, json value);

	static nlohmann::json GetSupercharge();

};


#endif //TXV_ECUI_LLSERVER_LLINTERFACE_H
