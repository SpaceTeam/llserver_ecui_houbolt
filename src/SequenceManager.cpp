#include "SequenceManager.h"

#include <iomanip>
#include <experimental/filesystem>

#include "utility/json.hpp"
#include "utility/utils.h"

#include "EcuiSocket.h"

std::string SequenceManager::configFilePath = "";

SequenceManager::~SequenceManager()
{
    if (isInitialized)
    {
        while (sequenceRunning || isAbortRunning)
        {
            Debug::print("wating for sequence to finish...");
            usleep(100000);
        }
    }
    if (sequenceThread.joinable())
    	sequenceThread.join();
}

void SequenceManager::plotMaps(uint8_t option=2)
{
    if (option != 1)
    {
        Debug::info("\n\nSENSOR RANGES PER NAME");
        for (const auto& item : sensorsNominalRangeMap)
        {
            Debug::info("================");
            Debug::info("Sensor:" + item.first);
            Debug::info("\tTime: [ValueMin, ValueMax]");
            Debug::info("----------------");
            for (const auto &itemTimestamp : item.second)
            {
                Debug::info("\t%d: [%.2f, %.2f]", itemTimestamp.first, itemTimestamp.second[0], itemTimestamp.second[1]);
            }

        }
        Debug::info("\n\nSENSOR RANGES PER TIME");
        for (const auto& item : sensorsNominalRangeTimeMap)
        {
            Debug::info("================");
            Debug::info("Time %d:", item.first);
            Debug::info("\tSensor: [ValueMin, ValueMax]");
            Debug::info("----------------");
            for (const auto &itemTimestamp : item.second)
            {
                Debug::info("\t" + itemTimestamp.first + ": [%.2f, %.2f]", itemTimestamp.second[0], itemTimestamp.second[1]);
            }
        }
    }

    if (option != 0)
    {
        Debug::info("\n\nDEVICE MAP PER NAME");
        for (const auto& item : deviceMap)
        {
            Debug::info("================");
            Debug::info("Output Device:" + item.first);
            Debug::info("\tTime: Value");
            Debug::info("----------------");
            for (const auto &itemTimestamp : item.second)
            {
                std::stringstream parametersString;
                std::copy(itemTimestamp.second.begin(), itemTimestamp.second.end(), std::ostream_iterator<double>(parametersString, ", "));
                Debug::info("\t%d: %s", itemTimestamp.first, parametersString.str().c_str());
            }

        }
    }

}

void SequenceManager::Init(Config &config)
{
    llInterface = LLInterface::Instance();
    eventManager = EventManager::Instance();

    autoAbortEnabled = config["/autoabort"];

    timerSyncInterval = (int32_t)(1000000.0/(double)config["/WEBSERVER/timer_sync_rate"]);

    configFilePath = config.getConfigFilePath();

    isInitialized = true;
	fileSystem = FileSystemAbstraction::Instance();
}

bool SequenceManager::GetAutoAbort()
{
    return autoAbortEnabled;
}

void SequenceManager::SetAutoAbort(bool active)
{
    if (!sequenceRunning)
    {
        autoAbortEnabled = active;
    }
}


void SequenceManager::AbortSequence(std::string abortMsg)
{
    if(sequenceRunning)
    {
        EcuiSocket::SendJson("abort", abortMsg);
        Debug::info("Aborting... " + abortMsg);

        sequenceToStop = true;
		Debug::print("Asked sequence to stop...");
		
	sequenceThread.join();

	while(sequenceRunning); // Todo probably remove since it does not work with thread
		
	abortSequence();
    }
    else
    {
        Debug::warning("cannot abort sequence: no running sequence");
    }
}

void SequenceManager::SetupLogging()
{
    time_t curr_time;
    tm * curr_tm;
    char dateTime_string[100];

    time(&curr_time);
    curr_tm = localtime(&curr_time);

    strftime(dateTime_string, 100, "%Y_%m_%d__%H_%M_%S", curr_tm);

    currentDirPath = "logs/" + std::string(dateTime_string);

    this->lastDir = currentDirPath;
    logFileName = std::string(dateTime_string) + ".csv";
    fileSystem->CreateDirectory("logs");
    fileSystem->CreateDirectory(currentDirPath);
    Debug::changeOutputFile(currentDirPath + "/" + std::string(dateTime_string) + ".csv");

    //save Sequence files
    fileSystem->SaveFile(currentDirPath + "/Sequence.json", jsonSequence.dump(4));
    fileSystem->SaveFile(currentDirPath + "/AbortSequence.json", jsonAbortSequence.dump(4));
    fileSystem->SaveFile(currentDirPath + "/comments.txt", comments);

    fileSystem->CopyFile(configFilePath, currentDirPath + "/");

}

void SequenceManager::WritePostSeqComment(std::string msg){
    fileSystem->SaveFile(lastDir + "/postseq-comments.txt", msg);
}

bool SequenceManager::LoadSequence(nlohmann::json jsonSeq)
{
    deviceMap.clear();
    sensorsNominalRangeMap.clear();
    sensorsNominalRangeTimeMap.clear();
    sequenceStartTime = INT64_MIN;
    sequenceStartTime = utils::toMicros(jsonSeq["globals"]["startTime"]);
    for (auto dataItem : jsonSeq["data"])
    {
        double timeCmd = GetTimestamp(dataItem);
        int64_t timestampCmdMicros = utils::toMicros(timeCmd);

        for (auto actionItem : dataItem["actions"])
        {
            //convert timestamp of action
            if (actionItem["timestamp"].type() == nlohmann::json::value_t::string)
            {
                Debug::error("no strings in actionitems allowed");
                AbortSequence("no strings as timestamp in action items allowed");
                return false;
            }

            double time = GetTimestamp(actionItem);
            int64_t timestampMicros = utils::toMicros(time);
            if (timestampMicros < 0)
            {
                Debug::error("timestamp in action must be positive");
                AbortSequence("timestamp in action must be positive");
                return false;
            }

            timestampMicros += timestampCmdMicros;

            actionItem.erase("timestamp");
            for (auto it = actionItem.begin(); it != actionItem.end(); ++it)
            {
                if (it.key().compare("sensorsNominalRange") == 0)
                {
                    nlohmann::json sensorsRanges = it.value();
                    for (auto sensorsIt = sensorsRanges.begin(); sensorsIt != sensorsRanges.end(); ++sensorsIt)
                    {
                        if (sensorsIt.value().type() == nlohmann::json::value_t::array && sensorsIt.value().size() == 2)
                        {
                            if(jsonSeq["globals"]["ranges"].contains(sensorsIt.key())) {
                                sensorsNominalRangeMap[sensorsIt.key()][timestampMicros][0] = sensorsIt.value()[0];
                                sensorsNominalRangeMap[sensorsIt.key()][timestampMicros][1] = sensorsIt.value()[1];
                                sensorsNominalRangeTimeMap[timestampMicros][sensorsIt.key()][0] = sensorsIt.value()[0];
                                sensorsNominalRangeTimeMap[timestampMicros][sensorsIt.key()][1] = sensorsIt.value()[1];
                            } else{
                                Debug::warning("Sensor Ranges set for %s but not defined in globals", sensorsIt.key().c_str());
                            }
                        }
                        else
                        {
                            Debug::error("Range of " + sensorsIt.key() + " not valid");
                            return false;
                        }
                    }
                }
                else
                {
                    std::vector<double> parameters = it.value();
                    deviceMap[it.key()][timestampMicros] = parameters;
                }
            }

        }
    }
    plotMaps();
    return true;
}

void SequenceManager::StartSequence(nlohmann::json jsonSeq, nlohmann::json jsonAbortSeq, std::string comments)
{
    if (sequenceThread.joinable())
    	sequenceThread.join();
    if (!sequenceRunning && !isAbortRunning)
    {
        jsonSequence = jsonSeq;
        jsonAbortSequence = jsonAbortSeq;
        SequenceManager::comments = comments;

        if (LoadSequence(jsonSeq))
        {
            SetupLogging();

            std::string msg;
            //get sensor names
            // std::map<std::string, std::tuple<double, uint64_t>> sensorData = LLInterface::GetLatestSensorData();
            // string msg;
            // for (int i = 0; i < sensorNames.size(); i++)
            // {
            //     msg += sensorNames[i] + ";";
            // }
            // msg += "Status;";
            msg += "SequenceTime;";
            for (const auto& rangeName: sensorsNominalRangeMap)
            {
                Debug::info("Sensor nominal range found: %s", ((std::string) rangeName.first).c_str());
                msg += rangeName.first + "Min;";
                msg += rangeName.first + "Max;";
            }
            for (auto &item : deviceMap)
            {
                msg += item.first + ";";
            }

            Debug::log(/*"Timestep;" +*/ msg + "\n");

            LoadInterpolationMap();

            startTime_us = utils::toMicros(jsonSeq["globals"]["startTime"]);
            endTime_us = utils::toMicros(jsonSeq["globals"]["endTime"]);
            int64_t interval_us = utils::toMicros(jsonSeq["globals"]["interval"]);
            Debug::info("%d %d %d", startTime_us, endTime_us, interval_us);

            EcuiSocket::SendJson("timer-start");

            sequenceRunning = true;
            sequenceThread = std::thread(&SequenceManager::sequenceLoop, this, interval_us);
            //sequenceThread.detach();
            std::string sequence_name = jsonSeq["data"][0]["desc"];

            Debug::print("Sequence Started " + sequence_name);
        }

    }
}

void SequenceManager::LoadInterpolationMap()
{
    for (auto it = jsonSequence["globals"]["interpolation"].begin(); it != jsonSequence["globals"]["interpolation"].end(); ++it)
    {
        std::string mode = it.value();
        if (mode.compare("none") == 0)
        {
            interpolationMap[it.key()] = Interpolation::NONE;
        }
        else if (mode.compare("linear") == 0)
        {
            interpolationMap[it.key()] = Interpolation::LINEAR;
        }
        else
        {
            interpolationMap[it.key()] = Interpolation::NONE;
        }
        Debug::info("interpolation created for %s: mode %s", it.key().c_str(), mode.c_str());
    }

}

void SequenceManager::CheckSensors(int64_t microTime)
{
	if (!autoAbortEnabled) {
		return;
	}
    std::map<std::string, std::tuple<double, uint64_t>> sensors = llInterface->GetLatestSensorData();

    for (const auto& sensor : sensors)
    {
        if (autoAbortEnabled && (sensorsNominalRangeMap.find(sensor.first) != sensorsNominalRangeMap.end()))
        {
            auto currInterval = sensorsNominalRangeMap[sensor.first].begin();
            double currValue = std::get<0>(sensor.second);
            if (currValue < currInterval->second[0])
            {
                std::stringstream stream;
                stream << std::fixed << "auto abort Sensor: " << sensor.first << " value " + std::to_string(currValue) << " too low" << " at Time " << std::setprecision(2) << ((microTime/1000)/1000.0) << " seconds";
                std::string abortMsg = stream.str();
                if (sequenceRunning)
                {
                    AbortSequence(abortMsg);
                }
            }
            else if (currValue > currInterval->second[1])
            {
                std::stringstream stream;
                stream << std::fixed << "auto abort Sensor: " << sensor.first << " value " + std::to_string(currValue) << " too high" << " at Time " << std::setprecision(2) << ((microTime/1000)/1000.0) << " seconds";
                std::string abortMsg = stream.str();
                if (sequenceRunning)
                {
                    AbortSequence(abortMsg);
                }
            }
        }
    }

}

double SequenceManager::GetTimestamp(nlohmann::json obj)
{
    //convert timestamp of action
    double time = 0.0;
    if (utils::keyExists(obj, "timestamp"))
    {
        if (obj["timestamp"].type() == nlohmann::json::value_t::string)
        {
            std::string timeStr = obj["timestamp"];
            if (timeStr.compare("START") == 0)
            {
                time = jsonSequence["globals"]["startTime"];
            }
            else if (timeStr.compare("END") == 0)
            {
                time = jsonSequence["globals"]["endTime"];
            } else {
	            Debug::error("in GetTimestamp: string timestamps have to be START or END. It was: %s", obj["timestamp"]);
            }
        } else if (obj["timestamp"].type() == nlohmann::json::value_t::number_float
                   || obj["timestamp"].type() == nlohmann::json::value_t::number_integer
                   || obj["timestamp"].type() == nlohmann::json::value_t::number_unsigned) {
	        time = obj["timestamp"];
        } else {
        	Debug::error("in SequenceManager GetTimestamp: timestamp has to be a string or a number. It was: %s",obj["timestamp"]);
        }
    }
    else {
	    Debug::error("in SequenceManager GetTimestamp: timestamp key of object does not exist");
    }

    return time;
}

void SequenceManager::sequenceLoop(int64_t interval_us)
{
	struct sched_param param;
	param.sched_priority = 40;
	sched_setscheduler(0, SCHED_FIFO, &param);

	LoopTimer sequenceLoopTimer(interval_us, "sequenceThread");
	sequenceLoopTimer.init();

	while(!sequenceToStop)
	{
		sequenceLoopTimer.wait();

		int64_t sequenceTime_us = sequenceLoopTimer.getTimeElapsed_us() + startTime_us;

		if(sequenceTime_us > endTime_us)
		{
			EcuiSocket::SendJson("timer-done");
			sequenceToStop = true;
			break;
		}

		static int64_t nextTimePrint_us = startTime_us;
		if(sequenceTime_us >= nextTimePrint_us)
		{
			Debug::info("Sequence Time: %dus", sequenceTime_us);
			nextTimePrint_us += 300000;
		}

		static int64_t nextTimerSync_us = startTime_us;
		if(sequenceTime_us >= nextTimerSync_us)
		{
			EcuiSocket::SendJson("timer-sync", ((sequenceTime_us/1000) / 1000.0));
			nextTimerSync_us += timerSyncInterval;
		}

		std::string msg = std::to_string(sequenceTime_us / 1000000.0) + ";";
		syncMtx.lock();

		//log nominal ranges

        for (const auto &sensor : sensorsNominalRangeMap)
		{
			msg += std::to_string(sensor.second.begin()->second[0]) + ";";
			msg += std::to_string(sensor.second.begin()->second[1]) + ";";
		}

		bool shallExec;
		std::vector<double> nextValue = {0};

		for (const auto &devItem : deviceMap)
		{

			shallExec = true;

			if (devItem.second.size() > 1)
			{
				auto prevIt = devItem.second.begin();
				auto nextIt = std::next(devItem.second.begin());

				if (sequenceTime_us >= nextIt->first)
				{
					nextValue = nextIt->second;
					deviceMap[devItem.first].erase(deviceMap[devItem.first].begin());
				}
				else
				{
					Interpolation inter = Interpolation::NONE;
					if (interpolationMap.find(devItem.first) == interpolationMap.end())
					{
						Debug::error("%s not found in interpolation map, falling back to no interpolation", devItem.first.c_str());
					}
					else
					{
						inter = interpolationMap[devItem.first];
					}
					switch (inter)
					{
						case Interpolation::LINEAR:
						{
							nextValue = prevIt->second;
							double scale = ((nextIt->second[0] - prevIt->second[0]) * 1.0) / (nextIt->first - prevIt->first);
							nextValue[0] = (scale * (sequenceTime_us - prevIt->first)) + prevIt->second[0];
							break;
						}
						case Interpolation::NONE:
						default:
							nextValue = prevIt->second;
							if (sequenceStartTime != prevIt->first)
							{
								shallExec = false;
							}
					}
				}

				if (shallExec)
				{
					try
					{
						//Debug::error("%d: %s, %f", microTime, devItem.first.c_str(), nextValue[0]);
						eventManager->ExecuteCommand(devItem.first, nextValue, false);
					}
					catch(const std::exception& e)
					{
						Debug::error("SequenceManager::sequenceLoop ExecuteCommand error: %s", e.what());
					}
				}
			}
			else
			{
				Debug::info("no interval found, keeping the same value");
			}

			std::stringstream nextValueStringStream;
			std::copy(nextValue.begin(), nextValue.end(), std::ostream_iterator<double>(nextValueStringStream, ", "));
			msg += "[" + nextValueStringStream.str() + "];";
		}

		//delete depricated timestamp and update sensorsNominalRangeMap as well
		if (sensorsNominalRangeTimeMap.size() > 1)
		{
			auto beginRangeIt = sensorsNominalRangeTimeMap.begin();
			int64_t nextRangeChange = beginRangeIt->first;
			if (sequenceTime_us >= nextRangeChange)
			{
				for (const auto& sensor : beginRangeIt->second)
				{
					sensorsNominalRangeMap[sensor.first].erase(sensorsNominalRangeMap[sensor.first].begin());
				}
				sensorsNominalRangeTimeMap.erase(beginRangeIt);
				//plotMaps();
				Debug::info("updated sensor ranges at time: %d in ms", sequenceTime_us/1000);
			}
		}

		CheckSensors(sequenceTime_us);

		syncMtx.unlock();
		Debug::log(msg + "\n");
	}
	sequenceToStop = false;

	Debug::info("Sequence ended");

    Debug::flush();

	sequenceRunning = false;
}

void SequenceManager::abortSequence()
{	
    if (!sequenceRunning && !isAbortRunning)
    {
    	isAbortRunning = true;

        syncMtx.lock();

        for (auto it = jsonAbortSequence["actions"].begin(); it != jsonAbortSequence["actions"].end(); ++it)
        {
            if (it.key().compare("timestamp") != 0)
            {
            	std::vector<double> valueList = it.value();
                //TODO: potential undefined state when exception is thrown
                try
                {
                    eventManager->ExecuteCommand(it.key(), valueList, false);
                }
                catch(const std::exception& e)
                {
                    Debug::error("Error in AbortSequence, ignoring command...");
                }
            }
        }
        syncMtx.unlock();

		Debug::print("Abort Sequence Done");
        Debug::flush();
		isAbortRunning = false;
    }

}

bool SequenceManager::IsSequenceRunning()
{
    return sequenceRunning || isAbortRunning;
}
