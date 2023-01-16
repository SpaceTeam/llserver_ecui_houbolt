#include "LLInterface.h"

#include <regex>
#include <math.h>
#include <utility/utils.h>

#include "EcuiSocket.h"


void LLInterface::CalcThrustTransformMatrix()
{
    double deg60 = 60*M_PI/180;
    double beta = thrustVariables["beta"];
    double gamma = thrustVariables["gamma"];
    double alpha = thrustVariables["alpha"];
    double r = thrustVariables["r"];
    double d = thrustVariables["d"];
    thrustTransformMatrix = new std::vector<std::vector<double>>
    {
		{-sin(deg60-beta)*sin(gamma), -sin(deg60+beta)*sin(gamma), -sin(beta)*sin(gamma), sin(beta)*sin(gamma), sin(deg60+beta)*sin(gamma), sin(deg60-beta)*sin(gamma)},
		{-cos(deg60-beta)*sin(gamma), cos(deg60+beta)*sin(gamma), cos(beta)*sin(gamma), cos(beta)*sin(gamma), cos(deg60+beta)*sin(gamma), -cos(deg60-beta)*sin(gamma)},
		{cos(gamma), cos(gamma), cos(gamma), cos(gamma), cos(gamma), cos(gamma)},
		{r*cos(deg60-alpha)*cos(gamma)-d*cos(deg60-beta)*sin(gamma), r*cos(deg60+alpha)*cos(gamma)-d*cos(deg60+beta)*sin(gamma), -r*cos(alpha)*cos(gamma)+d*cos(beta)*sin(gamma), -r*cos(alpha)*cos(gamma)+d*cos(beta)*sin(gamma), r*cos(deg60+alpha)*cos(gamma)-d*cos(deg60+beta)*sin(gamma), r*cos(deg60-alpha)*cos(gamma)-d*cos(deg60-beta)*sin(gamma)},
		{r*sin(deg60-alpha)*cos(gamma)-d*sin(deg60-beta)*sin(gamma), r*sin(deg60+alpha)*cos(gamma)-d*sin(deg60+beta)*sin(gamma), r*sin(alpha)*cos(gamma)-d*sin(beta)*sin(gamma), -r*sin(alpha)*cos(gamma)+d*sin(beta)*sin(gamma), -r*sin(deg60+alpha)*cos(gamma)+d*sin(deg60+beta)*sin(gamma), -r*sin(deg60-alpha)*cos(gamma)+d*sin(deg60-beta)*sin(gamma)},
		{-r*sin(beta-alpha), r*sin(beta-alpha), -r*sin(beta-alpha), r*sin(beta-alpha), -r*sin(beta-alpha), r*sin(beta-alpha)}
    };
}

void LLInterface::Init(Config &config)
{
    if (!isInitialized)
    {
        Debug::print("Initializing EventManager...");
        eventManager = EventManager::Instance();
        eventManager->Init(config);
        Debug::print("Initializing EventManager done\n");

        Debug::print("Initializing StateController...");
        stateController = StateController::Instance();
        stateController->Init(std::bind(&EventManager::OnStateChange, eventManager, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), config);
        Debug::print("Initializing StateController done\n");

        Debug::print("Initializing CANManager...");
        canManager = CANManager::Instance();
        canManager->Init(config);
        Debug::print("Initializing CANManager done\n");

        Debug::print("Initializing GUIMapping...");
        guiMapping = new JSONMapping(config.getMappingFilePath(), "GUIMapping");
        LoadGUIStates();
        Debug::print("GUIMapping initialized\n");

        Debug::print("Waiting for States to be initialized...");
        // stateController->WaitUntilStatesInitialized(); //TODO: uncomment when can interface works
        Debug::print("All States initialized\n");

        Debug::print("Initializing Thrust Matrix...");
        thrustVariables["alpha"] = config["/THRUST/alpha"];
        thrustVariables["beta"] = config["/THRUST/beta"];
        thrustVariables["gamma"] = config["/THRUST/gamma"];
        thrustVariables["d"] = config["/THRUST/d"];
        thrustVariables["r"] = config["/THRUST/r"];
        CalcThrustTransformMatrix();
        Debug::print("Initializing Thrust Matrix done\n");

        Debug::print("Initializing DataFilter...");
        double sensorsSmoothingFactor = config["/WEBSERVER/sensors_smoothing_factor"];
        dataFilter = new DataFilter(sensorsSmoothingFactor);
        Debug::print("Initializing DataFilter done\n");

        Debug::print("Starting filterSensorsThread...");
        uint32_t filterSensorsInterval = (uint32_t)(1e6 / (double)config["/LLSERVER/sensor_state_sampling_rate"]);
        filterSensorsRunning = true;
        filterSensorsThread = new std::thread(&LLInterface::filterSensorsLoop, this, filterSensorsInterval);
        Debug::print("FilterSensorsThread started\n");

        isInitialized = true;
    }
}

LLInterface::~LLInterface()
{
    if (isInitialized)
    {
        Debug::print("Stopping State transmission...");
        StopStateTransmission();

        Debug::print("Stopping filterSensorsThread...");
        filterSensorsRunning = false;
        if(filterSensorsThread->joinable()) filterSensorsThread->join();
        else Debug::warning("filterSensorsThread was not joinable.");
        delete filterSensorsThread;

        Debug::print("Deleting Data Filter...");
        delete dataFilter;

        Debug::print("Deleting GUI Mapping Manager...");
        delete guiMapping;

        Debug::print("Logged: %zd", StateController::Instance()->count);

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
                        uint64_t timestamp = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();

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

void LLInterface::StartStateTransmission(Config &config)
{
    if (!transmitStatesRunning)
    {
        Debug::print("Starting transmitStatesThread...");
        uint32_t transmitStatesInterval = (uint32_t)(1e6 / (double)config["/WEBSERVER/state_transmission_rate"]);
        transmitStatesRunning = true;
		transmitStatesThread = new std::thread(&LLInterface::transmitStatesLoop, this, transmitStatesInterval);
		Debug::print("TransmitStatesThread started\n");
    }
}

void LLInterface::StopStateTransmission()
{
    if (transmitStatesRunning)
    {
        transmitStatesRunning = false;
        if(transmitStatesThread->joinable()) transmitStatesThread->join();
        else Debug::warning("transmitStatesThread was not joinable.");
        delete transmitStatesThread;
    }
}

void LLInterface::transmitStatesLoop(uint32_t transmitStatesInterval)
{
	struct sched_param param;
	param.sched_priority = 40;
	sched_setscheduler(0, SCHED_FIFO, &param);

	LoopTimer transmitStatesLoopTimer(transmitStatesInterval, "transmitStatesThread");

	transmitStatesLoopTimer.init();

	while(transmitStatesRunning)
	{
		transmitStatesLoopTimer.wait();

		std::map<std::string, std::tuple<double, uint64_t>> states = stateController->GetDirtyStates();

		if (states.size() > 0)
		{
			uint64_t time_us = transmitStatesLoopTimer.getTimePoint_us();
			TransmitStates(time_us, states);
		}
	}

    Debug::print("Stopped State Timer...");
}

void LLInterface::ExecuteCommand(std::string &commandName, std::vector<double> &params, bool testOnly)
{
    eventManager->ExecuteCommand(commandName, params, testOnly);
}

std::map<std::string, command_t> LLInterface::GetCommands()
{
    return eventManager->GetCommands();
}

void LLInterface::filterSensorsLoop(uint32_t filterSensorsInterval)
{
	struct sched_param param;
	param.sched_priority = 40;
	sched_setscheduler(0, SCHED_FIFO, &param);

	LoopTimer filterSensorsLoopTimer(filterSensorsInterval, "filterSensorsThread");

	filterSensorsLoopTimer.init();

	while(filterSensorsRunning)
	{
		filterSensorsLoopTimer.wait();

		std::map<std::string, std::tuple<double, uint64_t>> rawSensors;
		rawSensors = canManager->GetLatestSensorData();

		std::map<std::string, std::tuple<double, uint64_t>> filteredSensors;
		filteredSensors = dataFilter->FilterData(rawSensors);

        if (filteredSensors.find("pmu_barometer:sensor") != filteredSensors.end())
	    {
	        uint64_t timestamp = utils::getCurrentTimestamp();
	        double baro = std::get<0>(filteredSensors["pmu_barometer:sensor"]);
	        double height = 0.3048 * (1 - pow((baro / 1013.25), 0.190284)) * 145366.45;
	        filteredSensors["pmu_altitude:sensor"] = {height, timestamp};
	    }

	    if (filteredSensors.find("rcu_barometer:sensor") != filteredSensors.end())
	    {
	        uint64_t timestamp = utils::getCurrentTimestamp();
	        double baro = std::get<0>(filteredSensors["rcu_barometer:sensor"]);
	        double height = 0.3048 * (1 - pow((baro / 1013.25), 0.190284)) * 145366.45;
	        filteredSensors["rcu_altitude:sensor"] = {height, timestamp};
	    }

        if (filteredSensors.find("lora:pmu_barometer:sensor") != filteredSensors.end())
        {
            uint64_t timestamp = utils::getCurrentTimestamp();
            double baro = std::get<0>(filteredSensors["lora:pmu_barometer:sensor"]);
            double height = 0.3048 * (1 - pow((baro / 1013.25), 0.190284)) * 145366.45;
            filteredSensors["lora:pmu_altitude:sensor"] = {height, timestamp};
        }

        if (filteredSensors.find("lora:rcu_barometer:sensor") != filteredSensors.end())
        {
            uint64_t timestamp = utils::getCurrentTimestamp();
            double baro = std::get<0>(filteredSensors["lora:rcu_barometer:sensor"]);
            double height = 0.3048 * (1 - pow((baro / 1013.25), 0.190284)) * 145366.45;
            filteredSensors["lora:rcu_altitude:sensor"] = {height, timestamp};
        }

        //TODO: reconsider if states should be iterated here or 
        //sent directly to a new setStates of the state controller
        for (auto &sensor : filteredSensors)
        {
            try
            {
                if (std::get<0>(stateController->GetState(sensor.first)) != std::get<0>(sensor.second))
                {
                    stateController->SetState(sensor.first, std::get<0>(sensor.second), std::get<1>(sensor.second));
                }
            }
            catch(const std::exception& e)
            {
                // assume state not in state controller yet
                stateController->SetState(sensor.first, std::get<0>(sensor.second), std::get<1>(sensor.second));
            }
            
            
            
        }

	}

	Debug::print("Stopped FilterSensorsThread");
}

std::map<std::string, std::tuple<double, uint64_t>> LLInterface::GetLatestSensorData()
{
    return canManager->GetLatestSensorData();
}

nlohmann::json LLInterface::StatesToJson(std::map<std::string, std::tuple<double, uint64_t>> &states)
{
    nlohmann::json statesJson = nlohmann::json::array();
    for (const auto& state : states)
    {
        nlohmann::json stateJson = nlohmann::json::object();

        stateJson["name"] = state.first;
        stateJson["value"] = std::get<0>(state.second);
        stateJson["timestamp"] = std::get<1>(state.second);

        statesJson.push_back(stateJson);
    }

    return statesJson;
}

nlohmann::json LLInterface::StatesToJson(std::map<std::string, std::tuple<double, uint64_t, bool>> &states)
{
    nlohmann::json statesJson = nlohmann::json::array();
    for (const auto& state : states)
    {
        nlohmann::json stateJson = nlohmann::json::object();

        stateJson["name"] = state.first;
        stateJson["value"] = std::get<0>(state.second);
        stateJson["timestamp"] = std::get<1>(state.second);

        statesJson.push_back(stateJson);
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

nlohmann::json LLInterface::GetAllStateLabels()
{
    std::map<std::string, std::tuple<double, uint64_t, bool>> states = stateController->GetAllStates();
    
    nlohmann::json statesJson = nlohmann::json::array();

    nlohmann::json *guiMappingJson = guiMapping->GetJSONMapping();
    for (const auto& state : states)
    {
        nlohmann::json stateJson = nlohmann::json::object();
        stateJson["name"] = state.first;
        stateJson["label"] = state.first;
        for (const auto& elem : *guiMappingJson)
        {
            if (state.first.compare(elem["state"]) == 0)
            {
                stateJson["label"] = elem["label"];
                break;
            }
        }
        
        statesJson.push_back(stateJson);
    }

    return statesJson;
}

nlohmann::json LLInterface::GetStates(nlohmann::json &stateNames)
{
    if (!stateNames.is_array())
    {
        throw std::runtime_error("LLInterface - GetStates: stateNames must be json array");
    }
    std::map<std::string, std::tuple<double, uint64_t, bool>> states;
    for (const auto& stateName : stateNames)
    {
        if (!stateName.is_string())
        {
            throw std::runtime_error("LLInterface - GetStates: stateName must be string");
        }
           
        states[stateName] = stateController->GetState(stateName);
    }
    nlohmann::json statesJson = StatesToJson(states);
    return statesJson;
    
}

void LLInterface::SetState(std::string stateName, double value, uint64_t timestamp)
{
    stateController->SetState(stateName, value, timestamp);
}
