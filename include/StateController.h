//
// Created by Markus on 31.03.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_STATECONTROLLER_H
#define LLSERVER_ECUI_HOUBOLT_STATECONTROLLER_H

#include "common.h"

#include <map>
#include <tuple>

class StateController
{
private:
    std::map<std::string, std::tuple<double, bool>> states;
    std::function<void(std::string, double)> onStateChangeCallback;

	static StateController* instance;
	bool initialized = false;
	StateController(const StateController& copy);
	StateController();
	~StateController();

public:
    void Init(std::function<void(std::string, double)> onStateChangeCallback);
    void AddStates(std::map<std::string, double> states);
    bool ChangeState(std::string stateName, double value);

    std::map<std::string, double> GetDirtyStates();
	std::map<std::string, std::tuple<double, bool>> GetAllStates();

	static StateController* Instance();

};

#endif //LLSERVER_ECUI_HOUBOLT_STATECONTROLLER_H
