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

#include "can/CANManager.h"
#include "DataFilter.h"
#include "EventManager.h"
#include "StateController.h"

class LLInterface
{

private:

    static I2C* i2cDevice;
    static WarnLight* warnLight;
    static TMPoE *tmPoE;
    static CANManager *canManager;
    static EventManager *eventManager;
    static StateController *stateController;

    static DataFilter *dataFilter;
    //static GPIO[] gpioDevices;

    //static SPI* spiDevice;

    static bool isInitialized;

    static bool useTMPoE;

    static bool isTransmittingStates;
    static int32_t warnlightStatus;
    static Timer* stateTimer;
    static Timer* sensorTimer;

    static void GetStates(int64_t microTime);
    static void StopGetStates();

	static void FilterSensors(int64_t microTime);
	static void StopFilterSensors();

    LLInterface();

    ~LLInterface();
public:

    static void Init();
    static void Destroy();

    static void TransmitStates(int64_t microTime, std::map<std::string, double> &states);

    static std::vector<std::string> GetAllSensorNames();
    static std::map<std::string, double> GetAllSensors();

    static std::vector<std::string> GetAllOutputNames();

    static void StartStateTransmission();
    static void StopStateTransmission();

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
