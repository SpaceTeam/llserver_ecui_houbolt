//
// Created by Markus on 03.04.21.
//

#include "StateController.h"

StateController* instance = nullptr;

StateController::~StateController()
{
    delete instance;
    initialized = false;
}

void StateController::Init(std::function<void(std::string, double)> onStateChangeCallback)
{
    if (!initialized)
    {
        this->onStateChangeCallback = onStateChangeCallback;
        initialized = true;
    }
}

void StateController::AddStates(std::map<std::string, double> states)
{
    for (auto& state : states)
    {
        this->states[state.first] = {state.second, false};
    }
}

bool StateController::ChangeState(std::string stateName, double value)
{
    try
    {
        auto& state = this->states[stateName];
        std::get<0>(state) = value;
        std::get<1>(state) = true;
        this->onStateChangeCallback(stateName, value);
    }
    catch (const std::exception& e)
    {
        Debug::error(e.what());
    }
}

std::map<std::string, double> StateController::GetDirtyStates()
{
    std::map<std::string, double> dirties;
    for (auto& state : this->states)
    {
        if (std::get<1>(state.second))
        {
            dirties[state.first] =  std::get<0>(state.second);
        }
    }

    return dirties;
}

std::map<std::string, std::tuple<double, bool>> StateController::GetAllStates()
{
    return states;
}

StateController *StateController::Instance()
{
    if (instance == nullptr)
    {
        instance = new StateController();
    }
    return instance;
}
