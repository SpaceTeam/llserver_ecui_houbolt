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

class StateController : public Singleton<StateController>
{
    friend class Singleton;
private:
    std::map<std::string, std::tuple<double, uint64_t, bool>> states;
    std::function<void(std::string, double)> onStateChangeCallback;

	bool initialized = false;

	std::mutex stateMtx;

    ~StateController();
public:

    //TODO: MP Maybe add timestamp to callback argument as well
    void Init(std::function<void(std::string, double)> onStateChangeCallback);

    /**
     * blocks until all map entries have a timestamp != 0
     * all states should already be added at this point, no checking if sates are added afterwards
     */
    void WaitUntilStatesInitialized();

    void AddUninitializedStates(std::vector<std::string> &states);
    void AddStates(std::map<std::string, std::tuple<double, uint64_t>> &states);
    void SetState(std::string stateName, double value, uint64_t timestamp);

    double GetStateValue(std::string stateName);
    std::map<std::string, std::tuple<double, uint64_t>> GetDirtyStates();
	std::map<std::string, std::tuple<double, uint64_t, bool>> GetAllStates();

};

#endif //LLSERVER_ECUI_HOUBOLT_STATECONTROLLER_H
