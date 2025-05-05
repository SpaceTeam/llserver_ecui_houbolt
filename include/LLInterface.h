#pragma once

#include <thread>
#include "common.h"

#include "utility/Singleton.h"

#include "utility/json.hpp"
#include "utility/JSONMapping.h"
#include "utility/Config.h"

#include "can/CANManager.h"
#include "DataFilter.h"
#include "EventManager.h"
#include "StateController.h"
#include "utility/LoopTimer.hpp"

class LLInterface : public Singleton<LLInterface>
{
    friend class Singleton;
    
	private:
		JSONMapping *guiMapping = nullptr;
		CANManager *canManager = nullptr;
		EventManager *eventManager = nullptr;
		StateController *stateController = nullptr;
		DataFilter *dataFilter = nullptr;

		bool isInitialized;

		std::thread* transmitStatesThread;
		bool transmitStatesRunning;
		void transmitStatesLoop(uint32_t transmitStatesInterval);

		std::thread* filterSensorsThread;
		bool filterSensorsRunning;
		void filterSensorsLoop(uint32_t filterSensorsInterval);

		std::map<std::string, double> thrustVariables;
		std::vector<std::vector<double>> *thrustTransformMatrix;

		void CalcThrustTransformMatrix();

		void LoadGUIStates();

		static nlohmann::json StatesToJson(std::map<std::string, std::tuple<double, uint64_t>> &states);
		static nlohmann::json StatesToJson(std::map<std::string, std::tuple<double, uint64_t, bool>> &states);

public:
		virtual ~LLInterface();

		virtual void Init(Config &config);

		virtual nlohmann::json GetGUIMapping();

		virtual void TransmitStates(int64_t microTime, std::map<std::string, std::tuple<double, uint64_t>> &states);

		virtual void StartStateTransmission(Config &config);
		virtual void StopStateTransmission();

		virtual nlohmann::json GetAllStates();
		virtual nlohmann::json GetAllStateLabels();

		virtual nlohmann::json GetStates(nlohmann::json &stateNames);
		virtual void SetState(std::string stateName, double value, uint64_t timestamp);

		virtual void ExecuteCommand(std::string &commandName, std::vector<double> &params, bool testOnly);
		virtual std::map<std::string, command_t> GetCommands();

		virtual std::map<std::string, std::tuple<double, uint64_t>> GetLatestSensorData();
};
