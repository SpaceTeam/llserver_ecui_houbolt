//
// Created by Markus on 2019-10-15.
//

#ifndef TXV_ECUI_LLSERVER_LLINTERFACE_H
#define TXV_ECUI_LLSERVER_LLINTERFACE_H

#include "common.h"
#include "drivers/I2C.h"

#include "json.hpp"

#include "drivers/WarnLight.h"
#include "drivers/TmPoE.h"

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
    static int32_t warnlightStatus;
    static Timer* sensorTimer;

	static double sensorsSmoothingFactor;
	static std::map<std::string, double> filteredSensorBuffer;

    static void GetSensors(int64 microTime);
    static void StopGetSensors();

	static void FilterSensors(std::map<std::string, double> rawSensors);

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
    static void UpdateWarningLight(std::map<std::string, double> sensors={});


    static bool ExecCommand(std::string name, json value);

	static nlohmann::json GetSupercharge();

};


#endif //TXV_ECUI_LLSERVER_LLINTERFACE_H
