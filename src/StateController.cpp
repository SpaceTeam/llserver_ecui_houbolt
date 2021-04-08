//
// Created by Markus on 03.04.21.
//

#include "StateController.h"

StateController::~StateController()
{
    initialized = false;
}

void StateController::Init(std::function<void(std::string, double)> onStateChangeCallback)
{
    if (!initialized)
    {
        this->onStateChangeCallback = std::move(onStateChangeCallback);
        initialized = true;
    }
}

void StateController::WaitUntilStatesInitialized()
{
    bool done = false;
    bool aborted = false;
    while (!done)
    {
        for (auto& state : states)
        {
            if (std::get<1>(state.second) == 0)
            {
                aborted = true;
                break;
            }
        }
        if (aborted)
        {
            aborted = false;
        }
        else
        {
            done = true;
        }
    }

}

void StateController::AddUninitializedStates(std::vector<std::string> &states)
{
    for (std::string &state : states)
    {
        this->states[state] = {0.0, 0, false};
    }
}

void StateController::AddStates(std::map<std::string, std::tuple<double, uint64_t>> &states)
{
    for (auto& state : states)
    {
        this->states[state.first] = {std::get<0>(state.second), std::get<1>(state.second), false};
    }
}

void StateController::SetState(std::string stateName, double value, uint64_t timestamp)
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
