#pragma once

#include "common.h"
#include <atomic>

#include "utility/json.hpp"
#include "utility/Logging.h"
#include "utility/Config.h"

#include "LLInterface.h"
#include "EventManager.h"

#include "StateController.h"

typedef struct point_s
{
    int64_t x;
    int64_t y;
} Point;

typedef enum class interpolation_e
{
    NONE,
    LINEAR
} Interpolation;

class SequenceManager : public Singleton<SequenceManager>
{
    friend class Singleton;

	public:
		~SequenceManager();

		void Init(Config &config);

		bool GetAutoAbort();
		void SetAutoAbort(bool active);

		void StartSequence(nlohmann::json jsonSeq, nlohmann::json jsonAbortSeq, std::string comments);
		void AbortSequence(std::string abortMsg="abort");

		void WritePostSeqComment(std::string msg);

		bool IsSequenceRunning();

	private:
		static std::string configFilePath;

		void SetupLogging();

		void LoadInterpolationMap();
		bool LoadSequence(nlohmann::json jsonSeq);

		void CheckSensors(int64_t microTime);

		double GetTimestamp(nlohmann::json obj);

		void abortSequence();

		void plotMaps(uint8_t option);

		bool sequenceRunning = false;
		bool sequenceToStop = false;

		std::atomic_bool autoAbortEnabled = true;

		bool isAbortRunning = false;

		std::mutex syncMtx;

		int64_t startTime_us;
		int64_t endTime_us;

		void sequenceLoop(int64_t interval_us);

		nlohmann::json jsonSequence = nlohmann::json::object();
		nlohmann::json jsonAbortSequence = nlohmann::json::object();

		std::string comments;
		std::string currentDirPath;
		std::string logFileName;
		std::string lastDir;

		std::atomic_bool isInitialized = false;

		//config variables
		int32_t timerSyncInterval = 0;
		//----

		std::map<std::string, Interpolation> interpolationMap;
		std::map<int64_t, std::map<std::string, double[2]>> sensorsNominalRangeTimeMap;
		std::map<std::string, std::map<int64_t, double[2]>> sensorsNominalRangeMap;
		std::map<std::string, std::map<int64_t, std::vector<double>>> deviceMap;
		int64_t sequenceStartTime = INT64_MIN;

		LLInterface *llInterface = nullptr;
		EventManager *eventManager = nullptr;
		
		std::thread sequenceThread;
};
