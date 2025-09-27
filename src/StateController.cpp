//
// Created by Markus on 03.04.21.
//

#include "StateController.h"
#include "utility/Debug.h"
#include "tracepoint_wrapper.h"

StateController::~StateController()
{
    initialized = false;

    if (logger != nullptr) {
        delete logger;
    }
}

void StateController::Init(std::function<void(std::string, double, double)> onStateChangeCallback, Config &config)
{
    if (!initialized)
    {
        this->onStateChangeCallback = std::move(onStateChangeCallback);
#ifndef NO_INFLUX
        logger = new InfluxDbLogger();
        logger->Init(config["/INFLUXDB/database_ip"],
                     config["/INFLUXDB/database_port"],
                     config["/INFLUXDB/database_name"],
                     config["/INFLUXDB/state_measurement"], MICROSECONDS,
                     config["/INFLUXDB/buffer_size"]);
#endif
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
    LLSERVER_INSTRUMENTED_LOCK(stateMtx, llserver::trace::MUTEX_STATE_ADD_UNINIT);
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
    LLSERVER_INSTRUMENTED_LOCK(stateMtx, llserver::trace::MUTEX_STATE_ADD_STATES);
    for (auto& state : states)
    {
        this->states[state.first] = {std::get<0>(state.second), std::get<1>(state.second), false};
    }
}

std::tuple<double, uint64_t, bool> StateController::GetState(std::string stateName)
{
    LLSERVER_INSTRUMENTED_LOCK(stateMtx, llserver::trace::MUTEX_STATE_GET_STATE);
    std::tuple<double, uint64_t, bool> value;
    try
    {
        value = states.at(stateName);
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("StateController - GetState: " + std::string(e.what()));
    }
    return value;
}

void StateController::SetState(std::string stateName, double value, uint64_t timestamp)
{
    try
    {
        double oldValue;
        bool firstEntry = false;
        {
            LLSERVER_INSTRUMENTED_LOCK(stateMtx, llserver::trace::MUTEX_STATE_SET_STATE);

            if (this->states.find(stateName) == this->states.end())
            {
                firstEntry = true;
            }
            auto *state = &this->states[stateName]; //this inits tuple to 0s when statename not in states
            if (firstEntry)
            {
                oldValue = NAN;
            }
            else
            {
                oldValue = std::get<0>(*state);
            }
            
            std::get<0>(*state) = value;
            std::get<1>(*state) = timestamp;
            std::get<2>(*state) = true;
            //Debug::print("%zd: %s, %zd", count, stateName.c_str(), count);
#ifndef NO_INFLUX
            logger->log(stateName, value, timestamp);
#endif
            if(timestamp != 0) {
                count++;
            }
        }
        this->onStateChangeCallback(stateName, oldValue, value);
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("StateController - ChangeState: " + std::string(e.what()));
    }
}

double StateController::GetStateValue(std::string stateName)
{
    LLSERVER_INSTRUMENTED_LOCK(stateMtx, llserver::trace::MUTEX_STATE_GET_VALUE);
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
    LLSERVER_INSTRUMENTED_LOCK(stateMtx, llserver::trace::MUTEX_STATE_GET_DIRTY);
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
    LLSERVER_INSTRUMENTED_LOCK(stateMtx, llserver::trace::MUTEX_STATE_GET_ALL);
    std::map<std::string, std::tuple<double, uint64_t, bool>> statesCopy;
    statesCopy.insert(states.begin(), states.end());
    return statesCopy;
}
