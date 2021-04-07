//
// Created by Markus on 03.04.21.
//

#include "StateController.h"

StateController* StateController::instance = nullptr;

StateController::~StateController()
{
    delete instance;
    initialized = false;
}

StateController *StateController::Instance()
{
    if (instance == nullptr)
    {
        instance = new StateController();
    }
    return instance;
}

void StateController::Init(std::function<void(std::string, double)> onStateChangeCallback)
{
    if (!initialized)
    {
        this->onStateChangeCallback = onStateChangeCallback;
        initialized = true;
    }
}

void StateController::AddStates(std::map<std::string, std::tuple<double, uint64_t>> &states)
{
    for (auto& state : states)
    {
        this->states[state.first] = {std::get<0>(state.second), std::get<1>(state.second), false};
    }
}

void StateController::ChangeState(std::string stateName, double value, uint64_t timestamp)
{
    try
    {
        auto& state = this->states[stateName];
        std::get<0>(state) = value;
        std::get<1>(state) = timestamp;
        std::get<2>(state) = true;
        this->onStateChangeCallback(stateName, value);
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("StateController - ChangeState: " + std::string(e.what()));
    }
}

double StateController::GetStateValue(std::string stateName)
{
    double value;
    try
    {
        value = std::get<0>(states[stateName]);
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("StateController - GetStateValue: " + std::string(e.what()));
    }
    return value;
}

std::map<std::string, std::tuple<double, uint64_t>> StateController::GetDirtyStates()
{
    std::map<std::string, std::tuple<double, uint64_t>> dirties;
    for (auto& state : this->states)
    {
        if (std::get<2>(state.second))
        {
            dirties[state.first] =  {std::get<0>(state.second), std::get<1>(state.second)};
            std::get<2>(state.second) = false;
        }
    }

    return dirties;
}

std::map<std::string, std::tuple<double, uint64_t, bool>> StateController::GetAllStates()
{
    return states;
}
