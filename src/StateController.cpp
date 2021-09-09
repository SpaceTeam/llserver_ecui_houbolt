//
// Created by Markus on 03.04.21.
//

#include "StateController.h"
#include "utility/Config.h"

StateController::~StateController()
{
    initialized = false;

    if (logger != nullptr) {
        delete logger;
    }
}

void StateController::Init(std::function<void(std::string, double, double)> onStateChangeCallback)
{
    if (!initialized)
    {
        this->onStateChangeCallback = std::move(onStateChangeCallback);
        logger = new InfluxDbLogger();
        logger->Init(std::get<std::string>(Config::getData("INFLUXDB/database_ip")),
                     std::get<int>(Config::getData("INFLUXDB/database_port")),
                     std::get<std::string>(Config::getData("INFLUXDB/database_name")),
                     std::get<std::string>(Config::getData("INFLUXDB/state_measurement")), MICROSECONDS,
                     std::get<int>(Config::getData("INFLUXDB/buffer_size")));
        //logger->Init("127.0.0.1", 8086, "testDb", "states", MICROSECONDS, 65536);
        initialized = true;
    }
}

void StateController::WaitUntilStatesInitialized()
{
    bool done = false;
    bool aborted = false;
    while (!done)
    {
        stateMtx.lock();
        for (auto& state : states)
        {
            if (std::get<1>(state.second) == 0)
            {
                aborted = true;
                break;
            }
        }
        stateMtx.unlock();
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
    std::lock_guard<std::mutex> lock(stateMtx);
    for (std::string &state : states)
    {
        this->states[state] = {0.0, 0, false};
    }
}

/**
 * add states to the state controller, currently does not trigger on change event
 * @param states
 */
void StateController::AddStates(std::map<std::string, std::tuple<double, uint64_t>> &states)
{
    std::lock_guard<std::mutex> lock(stateMtx);
    for (auto& state : states)
    {
        this->states[state.first] = {std::get<0>(state.second), std::get<1>(state.second), false};
    }
}

void StateController::SetState(std::string stateName, double value, uint64_t timestamp)
{
    try
    {
        stateMtx.lock();
        //TODO: I don't understand how that works when state is not in map, but it's magic appearently
        auto *state = &this->states[stateName];
        double oldValue = std::get<0>(*state);
        std::get<0>(*state) = value;
        std::get<1>(*state) = timestamp;
        std::get<2>(*state) = true;
        logger->log(stateName, value, timestamp);
        stateMtx.unlock();
        this->onStateChangeCallback(stateName, oldValue, value);
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("StateController - ChangeState: " + std::string(e.what()));
    }
}

double StateController::GetStateValue(std::string stateName)
{
    std::lock_guard<std::mutex> lock(stateMtx);
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
    std::lock_guard<std::mutex> lock(stateMtx);
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
    std::lock_guard<std::mutex> lock(stateMtx);
    std::map<std::string, std::tuple<double, uint64_t, bool>> statesCopy;
    statesCopy.insert(states.begin(), states.end());
    return statesCopy;
}
