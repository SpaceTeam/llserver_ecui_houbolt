//
// Created by Markus on 31.03.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_STATECONTROLLER_H
#define LLSERVER_ECUI_HOUBOLT_STATECONTROLLER_H

#include <map>
#include <tuple>
#include <vector>
#include <functional>
#include <string>

#include "common.h"

#include "utility/Singleton.h"
#include "utility/Config.h"

#include "logging/InfluxDbLogger.h"

class StateController : public Singleton<StateController>
{
    friend class Singleton;
private:
    std::map<std::string, std::tuple<double, uint64_t, bool>> states;
    std::function<void(std::string, double, double)> onStateChangeCallback;

	bool initialized = false;

	std::mutex stateMtx;

    InfluxDbLogger *logger = nullptr;

    ~StateController();
public:

    std::size_t count = 0;
    //TODO: MP Maybe add timestamp to callback argument as well
    void Init(std::function<void(std::string, double, double)> onStateChangeCallback, Config &config);

    /**
     * blocks until all map entries have a timestamp != 0
     * all states should already be added at this point, no checking if sates are added afterwards
     */
    void WaitUntilStatesInitialized();

    void AddUninitializedStates(std::vector<std::string> &states);
    void AddStates(std::map<std::string, std::tuple<double, uint64_t>> &states);
    std::tuple<double, uint64_t, bool> GetState(std::string stateName);
    void SetState(std::string stateName, double value, uint64_t timestamp);

    double GetStateValue(std::string stateName);
    std::map<std::string, std::tuple<double, uint64_t>> GetDirtyStates();
	std::map<std::string, std::tuple<double, uint64_t, bool>> GetAllStates();

};

#endif //LLSERVER_ECUI_HOUBOLT_STATECONTROLLER_H
