#include <regex>
#include <math.h>
#include <utility/utils.h>
#include <chrono>

#include "EcuiSocket.h"
#include "utility/Config.h"

#include "LLInterface.h"

#define PI 3.14159265

void LLInterface::CalcThrustTransformMatrix()
{
    double deg60 = 60*PI/180;
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

void LLInterface::Init()
{
    if (!isInitialized)
    {
        Debug::print("Initializing EventManager...");
        eventManager = EventManager::Instance();
        eventManager->Init();
        Debug::print("Initializing EventManager done\n");

        Debug::print("Initializing StateController...");
        stateController = StateController::Instance();
        stateController->Init(std::bind(&EventManager::OnStateChange, eventManager, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        Debug::print("Initializing StateController done\n");

        Debug::print("Initializing CANManager...");
        canManager = CANManager::Instance();
        canManager->Init();
        Debug::print("Initializing CANManager done\n");

        Debug::print("Initializing GUIMapping...");
        guiMapping = new JSONMapping(Config::getMappingFilePath(), "GUIMapping");
        LoadGUIStates();
        Debug::print("GUIMapping initialized\n");

        Debug::print("Waiting for States to be initialized...");
        // stateController->WaitUntilStatesInitialized(); //TODO: uncomment when can interface works
        Debug::print("All States initialized\n");

        Debug::print("Initializing Thrust Matrix...");
        thrustVariables["alpha"] = std::get<double>(Config::getData("THRUST/alpha"));
        thrustVariables["beta"] = std::get<double>(Config::getData("THRUST/beta"));
        thrustVariables["gamma"] = std::get<double>(Config::getData("THRUST/gamma"));
        thrustVariables["d"] = std::get<double>(Config::getData("THRUST/d"));
        thrustVariables["r"] = std::get<double>(Config::getData("THRUST/r"));
        CalcThrustTransformMatrix();
        Debug::print("Initializing Thrust Matrix done\n");

        Debug::print("Initializing DataFilter...");
        double sensorsSmoothingFactor = std::get<double>(Config::getData("WEBSERVER/sensors_smoothing_factor"));
        dataFilter = new DataFilter(sensorsSmoothingFactor);
        Debug::print("Initializing DataFilter done\n");

        Debug::print("Starting filterSensorsThread...");
        filterSensorsInterval = (uint64_t)(1e6 / std::get<double>(Config::getData("LLSERVER/sensor_state_sampling_rate")));
        filterSensorsIntervalChecker = new IntervalChecker((uint32_t)(filterSensorsInterval * 1.1), "filterSensorsThread");
        filterSensorsRunning = true;
        filterSensorsThread = new std::thread(&LLInterface::filterSensorsLoop, this);
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
        filterSensorsThread->join();
        delete filterSensorsThread;
        delete filterSensorsIntervalChecker;

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

void LLInterface::StartStateTransmission()
{
    if (!transmitStatesRunning)
    {
        Debug::print("Starting transmitStatesThread...");
        transmitStatesInterval = (uint64_t)(1e6 / std::get<double>(Config::getData("WEBSERVER/state_transmission_rate")));
        transmitStatesIntervalChecker = new IntervalChecker((uint32_t)(transmitStatesInterval * 1.1), "transmitStatesThread");
        transmitStatesRunning = true;
		transmitStatesThread = new std::thread(&LLInterface::transmitStatesLoop, this);
		Debug::print("TransmitStatesThread started\n");
    }
}

void LLInterface::StopStateTransmission()
{
    if (transmitStatesRunning)
    {
        transmitStatesRunning = false;
        transmitStatesThread->join();
        delete transmitStatesThread;
        delete transmitStatesIntervalChecker;
    }
}

void LLInterface::transmitStatesLoop()
{
	struct sched_param param;
	param.sched_priority = 40;
	sched_setscheduler(0, SCHED_FIFO, &param);

	auto nextTime = std::chrono::steady_clock::now();

	while(transmitStatesRunning)
	{
		transmitStatesIntervalChecker->check();

		std::map<std::string, std::tuple<double, uint64_t>> states = stateController->GetDirtyStates();

		if (states.size() > 0)
		{
			uint8_t time_us = std::chrono::time_point_cast<std::chrono::microseconds>(nextTime).time_since_epoch().count();
			TransmitStates(time_us, states);
		}

		nextTime += std::chrono::microseconds(transmitStatesInterval);
		std::this_thread::sleep_until(nextTime);
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

void LLInterface::filterSensorsLoop()
{
	struct sched_param param;
	param.sched_priority = 40;
	sched_setscheduler(0, SCHED_FIFO, &param);

	auto nextTime = std::chrono::steady_clock::now();

	while(filterSensorsRunning)
	{
		filterSensorsIntervalChecker->check();

		std::map<std::string, std::tuple<double, uint64_t>> rawSensors;
		rawSensors = canManager->GetLatestSensorData();

		std::map<std::string, std::tuple<double, uint64_t>> filteredSensors;
		filteredSensors = dataFilter->FilterData(rawSensors);

		//TODO: remove hotfix
		std::vector<std::vector<double>> sensorMatrix = {{0},{0},{0},{0},{0},{0}};
		std::vector<std::vector<double>> resultMatrix = {{0},{0},{0},{0},{0},{0}};
		std::string thrustState;
		double thrustSum = 0;
		for (int32_t i = 1; i < 7; i++)
		{
			thrustState = "engine_thrust_"+std::to_string(i)+":sensor";
			if (filteredSensors.find(thrustState) == filteredSensors.end())
			{
				break;
			}
			sensorMatrix[i-1][0] = std::get<0>(filteredSensors[thrustState]);
			thrustSum += sensorMatrix[i-1][0];
			if (i==6)
			{
				std::vector<std::vector<double>> transformMatrix = *thrustTransformMatrix;
				utils::matrixMultiply(transformMatrix, sensorMatrix, resultMatrix);
				uint64_t thrustTimestamp = utils::getCurrentTimestamp();
				filteredSensors["engine_thrust_x"] = {resultMatrix[0][0], thrustTimestamp};
				filteredSensors["engine_thrust_y"] = {resultMatrix[1][0], thrustTimestamp};
				filteredSensors["engine_thrust_z"] = {resultMatrix[2][0], thrustTimestamp};
				filteredSensors["engine_thrust_mx"] = {resultMatrix[3][0], thrustTimestamp};
				filteredSensors["engine_thrust_my"] = {resultMatrix[4][0], thrustTimestamp};
				filteredSensors["engine_thrust_mz"] = {resultMatrix[5][0], thrustTimestamp};
				filteredSensors["engine_thrust_sum"] = {thrustSum, thrustTimestamp};
			}
		}

		//TODO: reconsider if states should be iterated here or
		//sent directly to a new setStates of the state controller
		for (auto &sensor : filteredSensors)
		{
			stateController->SetState(sensor.first, std::get<0>(sensor.second), std::get<1>(sensor.second));
		}

		nextTime += std::chrono::microseconds(filterSensorsInterval);
		std::this_thread::sleep_until(nextTime);
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
