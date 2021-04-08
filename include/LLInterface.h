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

	static nlohmann::json StatesToJson(std::map<std::string, std::tuple<double, uint64_t>> &states);
	static nlohmann::json StatesToJson(std::map<std::string, std::tuple<double, uint64_t, bool>> &states);

    LLInterface();

    ~LLInterface();
public:

    static void Init();
    static void Destroy();

    static void TransmitStates(int64_t microTime, std::map<std::string, std::tuple<double, uint64_t>> &states);

    static void StartStateTransmission();
    static void StopStateTransmission();

    static nlohmann::json GetAllStates();
    static void SetState(std::string stateName, double value, uint64_t timestamp);

    static void TurnRed();
    static void TurnGreen();
    static void TurnYellow();
    static void BeepRed();

};


#endif //TXV_ECUI_LLSERVER_LLINTERFACE_H
