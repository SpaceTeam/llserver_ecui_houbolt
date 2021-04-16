//
// Created by Markus on 2019-10-15.
//

#ifndef TXV_ECUI_LLSERVER_LLINTERFACE_H
#define TXV_ECUI_LLSERVER_LLINTERFACE_H

#include "common.h"

#include "utility/Singleton.h"

#include "driver/I2C.h"

#include "utility/json.hpp"

#include "driver/WarnLight.h"
#include "driver/TmPoE.h"
#include "utility/JSONMapping.h"
#include "utility/Timer.h"

#include "can/CANManager.h"
#include "DataFilter.h"
#include "EventManager.h"
#include "StateController.h"

class LLInterface : public Singleton<LLInterface>
{

private:

    I2C* i2cDevice;
    WarnLight* warnLight;
    TMPoE *tmPoE;

    JSONMapping *guiMapping;
    CANManager *canManager;
    EventManager *eventManager;
    StateController *stateController;
    DataFilter *dataFilter;

    bool isInitialized;

    bool useTMPoE;

    bool isTransmittingStates;
    int32_t warnlightStatus;

    Timer* stateTimer;
    Timer* sensorTimer;

    void GetStates(int64_t microTime);
    void StopGetStates();

	void FilterSensors(int64_t microTime);
	void StopFilterSensors();

	void LoadGUIStates();

	static nlohmann::json StatesToJson(std::map<std::string, std::tuple<double, uint64_t>> &states);
	static nlohmann::json StatesToJson(std::map<std::string, std::tuple<double, uint64_t, bool>> &states);


public:

    void Init();
    ~LLInterface();

    nlohmann::json GetGUIMapping();

    void TransmitStates(int64_t microTime, std::map<std::string, std::tuple<double, uint64_t>> &states);

    void StartStateTransmission();
    void StopStateTransmission();

    nlohmann::json GetAllStates();
    void SetState(std::string stateName, double value, uint64_t timestamp);

    void TurnRed();
    void TurnGreen();
    void TurnYellow();
    void BeepRed();

};


#endif //TXV_ECUI_LLSERVER_LLINTERFACE_H
