//
// Created by Markus on 2019-10-15.
//

#include <regex>
#include <utility/utils.h>

//#include "hcp/HcpManager.h"
#include "EcuiSocket.h"
#include "utility/Timer.h"
#include "utility/Config.h"

#include "LLInterface.h"


void LLInterface::Init()
{
    if (!isInitialized)
    {
//        Debug::print("Initializing HcpManager...");
//        HcpManager::Init();
//        Debug::print("Initializing HcpManager done\n");
//        Debug::print("Starting periodic sensor fetching...");
//        HcpManager::StartSensorFetch(std::get<int>(Config::getData("HCP/sensor_sample_rate")));
//        Debug::print("Periodic sensor fetching started\n");

        Debug::print("Initializing EventManager...");
        eventManager = EventManager::Instance();
        eventManager->Init();
        Debug::print("Initializing EventManager done\n");

        Debug::print("Initializing StateController...");
        stateController = StateController::Instance();
        stateController->Init(std::bind(&EventManager::OnStateChange, eventManager, std::placeholders::_1, std::placeholders::_2));
        Debug::print("Initializing StateController done\n");

        Debug::print("Initializing CANManager...");
        canManager = CANManager::Instance();
        canManager->Init();
        Debug::print("Initializing CANManager done\n");

        Debug::print("Initializing GUIMapping...");
        std::string mappingPath = std::get<std::string>(Config::getData("mapping_path"));
        guiMapping = new JSONMapping(mappingPath, "GUIMapping");
        LoadGUIStates();
        Debug::print("GUIMapping initialized");

        Debug::print("Waiting for States to be initialized...");
        // stateController->WaitUntilStatesInitialized(); //TODO: uncomment when can interface works
        Debug::print("All States initialized\n");

        Debug::print("Initializing DataFilter...");
        double sensorsSmoothingFactor = std::get<double>(Config::getData("WEBSERVER/sensors_smoothing_factor"));
        dataFilter = new DataFilter(sensorsSmoothingFactor);
        Debug::print("Initializing DataFilter done\n");

        Debug::print("Starting Sensor State Timer...");
        stateTimer = new Timer(40, "stateTimer");
        sensorTimer = new Timer(40, "sensorTimer");
        uint64_t sensorSamplingRate = std::get<int>(Config::getData("LLSERVER/sensor_sampling_rate"));
        sensorTimer->startContinous(0, sensorSamplingRate,
                std::bind(&LLInterface::FilterSensors, this, std::placeholders::_1),
                std::bind(&LLInterface::StopFilterSensors, this));
        Debug::print("Connecting to warning light...");
        warnLight = new WarnLight(0);

        useTMPoE = std::get<bool>(Config::getData("useTMPoE"));
        if (useTMPoE)
        {
            Debug::print("Initializing TMPoE...");
            tmPoE = new TMPoE(0, 50);
        }
        //i2cDevice = new I2C(0, "someDev"); //not in use right now



        isInitialized = true;
    }
}



LLInterface::~LLInterface()
{
    if (isInitialized)
    {
        //Debug::print("Shutting down I2C...");
        //delete i2cDevice;
        Debug::print("Shutting down Debug...");
        delete warnLight;
        if (useTMPoE)
        {
            Debug::print("Shutting down Debug...");
            delete tmPoE;
        }

        Debug::print("Stopping State transmission...");
        StopStateTransmission();
        delete stateTimer;
        Debug::print("Stopping Sensor State Timer...");
        delete sensorTimer;

        Debug::print("Deleting Data Filter...");
        delete dataFilter;

        Debug::print("Deleting GUI Mapping Manager...");
        delete guiMapping;

        Debug::print("Shutting down CANManager...");
        CANManager::Destroy();

        Debug::print("Shutting down StateController...");
        StateController::Destroy();

        Debug::print("Shutting down EventManager...");
        EventManager::Destroy();
    }
}

/**
 * loads all states from the gui mapping
 * all gui mapping states must have a prefix named 'gui:' followed by at least one
 */
void LLInterface::LoadGUIStates()
{
    nlohmann::json *guiMappingJson = guiMapping->GetJSONMapping();

    try
    {

        std::map<std::string, std::tuple<double, uint64_t>> states;
        for (auto &group : guiMappingJson->items())
        {
            for (auto &elem : group.value()["elements"])
            {
                for (auto it = elem["states"].begin(); it != elem["states"].end(); ++it)
                {
                    std::string stateName = it.value()["stateName"];
                    if (std::regex_match(stateName, std::regex("gui:(\\w+)(:\\w+)*")))
                    {
                        double defaultValue = 0.0;
                        if (utils::keyExists(it.value(), "default"))
                        {
                            if (it.value()["default"].is_number())
                            {
                                defaultValue = it.value()["default"];
                            }
                            else
                            {
                                Debug::error("LLInterface - LoadGUIStates: default value in GUIMapping not a number, ignoring value");
                            }
                        }

                        auto now = std::chrono::high_resolution_clock::now();
                        uint64_t timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
                                now.time_since_epoch()).count();

                        states[stateName] = {defaultValue, timestamp};
                    }
                }
            }
        }

        stateController->AddStates(states);
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("LLInterface - LoadGUIStates:" + std::string(e.what()));
    }
}

nlohmann::json LLInterface::GetGUIMapping()
{
    return *guiMapping->GetJSONMapping();
}

void LLInterface::StartStateTransmission()
{
    if (!isTransmittingStates)
    {
        uint64_t stateSamplingRate = std::get<int>(Config::getData("LLSERVER/state_sampling_rate"));
        isTransmittingStates = true;
        stateTimer->startContinous(0, stateSamplingRate,
                std::bind(&LLInterface::GetStates, this, std::placeholders::_1),
                std::bind(&LLInterface::StopGetStates, this));
    }
}

void LLInterface::StopStateTransmission()
{
    if (isTransmittingStates)
    {

        stateTimer->stop();
        isTransmittingStates = false;
    }
}

void LLInterface::StopGetStates()
{
    Debug::print("Stopped State Timer...");
}

void LLInterface::GetStates(int64_t microTime)
{
    std::map<std::string, std::tuple<double, uint64_t>> states = stateController->GetDirtyStates();

    TransmitStates(microTime, states);

}

void LLInterface::FilterSensors(int64_t microTime)
{
    std::map<std::string, std::tuple<double, uint64_t>> rawSensors;
    rawSensors = canManager->GetLatestSensorData();

    std::map<std::string, std::tuple<double, uint64_t>> filteredSensors;
    filteredSensors = dataFilter->FilterData(rawSensors);



}

void LLInterface::StopFilterSensors()
{
    Debug::print("Stopped Sensor State Timer...");
}

nlohmann::json LLInterface::StatesToJson(std::map<std::string, std::tuple<double, uint64_t>> &states)
{
    nlohmann::json content = nlohmann::json::array();
    nlohmann::json statesJson;
    for (const auto& state : states)
    {
        nlohmann::json stateJson = nlohmann::json::object();

        stateJson["name"] = state.first;
        stateJson["value"] = std::get<0>(state.second);
        stateJson["timestamp"] = (double(std::get<1>(state.second))/1000.0);

        content.push_back(stateJson);
    }

    return statesJson;
}

nlohmann::json LLInterface::StatesToJson(std::map<std::string, std::tuple<double, uint64_t, bool>> &states)
{
    nlohmann::json content = nlohmann::json::array();
    nlohmann::json statesJson;
    for (const auto& state : states)
    {
        nlohmann::json stateJson = nlohmann::json::object();

        stateJson["name"] = state.first;
        stateJson["value"] = std::get<0>(state.second);
        stateJson["timestamp"] = (double(std::get<1>(state.second))/1000.0);

        content.push_back(stateJson);
    }

    return statesJson;
}

void LLInterface::TransmitStates(int64_t microTime, std::map<std::string, std::tuple<double, uint64_t>> &states)
{

    nlohmann::json statesJson = StatesToJson(states);
    EcuiSocket::SendJson("states", statesJson);
}

nlohmann::json LLInterface::GetAllStates()
{
    std::map<std::string, std::tuple<double, uint64_t, bool>> states = stateController->GetAllStates();
    nlohmann::json statesJson = StatesToJson(states);
    return statesJson;
}

void LLInterface::SetState(std::string stateName, double value, uint64_t timestamp)
{
    stateController->SetState(stateName, value, timestamp);
}

void LLInterface::TurnRed()
{
    warnLight->SetColor(255, 0, 0);
    warnLight->SetMode("default");
    warnLight->StopBuzzer();
}

void LLInterface::TurnGreen()
{
    warnLight->SetColor(0, 255, 0);
    warnLight->SetMode("spin");
    warnLight->StopBuzzer();
}

void LLInterface::TurnYellow()
{
    warnLight->SetColor(255, 255, 0);
    warnLight->SetMode("spin");
    warnLight->StopBuzzer();
}

void LLInterface::BeepRed()
{
    warnLight->SetColor(255, 0, 0);
    warnLight->SetMode("blink");
    warnLight->StartBuzzerBeep(500);
}
