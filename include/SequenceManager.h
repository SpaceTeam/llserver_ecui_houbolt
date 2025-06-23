#pragma once

#include "common.h"
#include <atomic>
#include <utility/FileSystemAbstraction.h>

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

		/**
		 * Interpolate linearly between start and end values.
		 *
		 * @param startTimeAndValues A tuple containing the start time and values.
		 *                           The first element is the start time,
		 *                           and the second element is a vector of values.
		 * @param endTimeAndValues   An optional tuple containing the end time and values.
		 *                           The first element is the end ,
		 *                           and the second element is a vector of values.
		 * @param currentTime        The current time.
		 * @return A tuple containing:
		 *         - A vector of interpolated values, if one could be computed.
		 *         - A boolean indicating whether the interpolation is complete. If true,
		 *           the end value should be used as the new start value for the next interpolation.
		 */
		static std::tuple<std::optional<std::vector<double>>, bool> interpolateLinear(
			const std::tuple<int64_t, std::vector<double>> &startTimeAndValues,
			const std::optional<std::tuple<int64_t, std::vector<double>>>& endTimeAndValues,
			int64_t currentTime
		);

		/**
		 * No interpolation. Conforms to the api of interpolateLinear.
		 */
		static std::tuple<std::optional<std::vector<double>>, bool> interpolateNone(
			const std::tuple<int64_t, std::vector<double>> &startTimeAndValues,
			const std::optional<std::tuple<int64_t, std::vector<double>>>& endTimeAndValues,
			int64_t currentTime
		);

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
		FileSystemAbstraction *fileSystem = nullptr;

		std::thread sequenceThread;
};
