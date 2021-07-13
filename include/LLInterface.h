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
    friend class Singleton;
private:

    I2C* i2cDevice = nullptr;
    WarnLight* warnLight = nullptr;
    TMPoE *tmPoE = nullptr;

    JSONMapping *guiMapping = nullptr;
    CANManager *canManager = nullptr;
    EventManager *eventManager = nullptr;
    StateController *stateController = nullptr;
    DataFilter *dataFilter = nullptr;

    bool isInitialized;

    bool useTMPoE;

    bool isTransmittingStates;
    int32_t warnlightStatus;

    Timer* stateTimer;
    Timer* sensorStateTimer;

    void GetStates(int64_t microTime);
    void StopGetStates();

	void FilterSensors(int64_t microTime);
	void StopFilterSensors();

	void LoadGUIStates();

	static nlohmann::json StatesToJson(std::map<std::string, std::tuple<double, uint64_t>> &states);
	static nlohmann::json StatesToJson(std::map<std::string, std::tuple<double, uint64_t, bool>> &states);

    ~LLInterface();
public:

    void Init();

    nlohmann::json GetGUIMapping();

    void TransmitStates(int64_t microTime, std::map<std::string, std::tuple<double, uint64_t>> &states);

    void StartStateTransmission();
    void StopStateTransmission();

    nlohmann::json GetAllStates();
    nlohmann::json GetAllStateLabels();
    void SetState(std::string stateName, double value, uint64_t timestamp);

    void ExecuteCommand(std::string &commandName, std::vector<double> &params, bool testOnly);
    std::map<std::string, command_t> GetCommands();

    void TurnRed();
    void TurnGreen();
    void TurnYellow();
    void BeepRed();

};


#endif //TXV_ECUI_LLSERVER_LLINTERFACE_H
