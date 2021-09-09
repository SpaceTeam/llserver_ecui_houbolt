//
// Created by Markus on 2019-09-27.
//
#include <iomanip>
// #include <filesystem>
#include <experimental/filesystem>

#include "utility/json.hpp"
#include "utility/utils.h"
#include "utility/Config.h"

#include "EcuiSocket.h"
#include "SequenceManager.h"

SequenceManager::~SequenceManager()
{
    if (isInitialized)
    {
        while (isRunning || isAbortRunning)
        {
            Debug::print("wating for sequence to finish...");
            usleep(100000);
        }
    }
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

void SequenceManager::Init()
{
    llInterface = LLInterface::Instance();
    eventManager = EventManager::Instance();

    timer = new Timer(40, "SequenceTimer");

    isAutoAbort = std::get<bool>(Config::getData("autoabort"));

    timerSyncInterval = 1000000/std::get<int>(Config::getData("WEBSERVER/timer_sync_rate"));

    isInitialized = true;
}


void SequenceManager::StopSequence()
{
    Debug::flush();
    Debug::print("Sequence Done");
    //TODO: MP Set warning light here or insert into sequence

    isRunning = false;
    if (!isAbort)
    {
        EcuiSocket::SendJson("timer-done");
    }
}

void SequenceManager::AbortSequence(std::string abortMsg)
{
    if (isRunning)
    {
        isAbort = true;
        timer->stop();

        EcuiSocket::SendJson("abort", abortMsg);
        Debug::error("Aborting... " + abortMsg);
        isRunning = false;

        StartAbortSequence();

        isAbort = false;
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
    std::experimental::filesystem::create_directory(currentDirPath);
    Debug::changeOutputFile(currentDirPath + "/" + std::string(dateTime_string) + ".csv");

    //save Sequence files
    utils::saveFile(currentDirPath + "/Sequence.json", jsonSequence.dump(4));
    utils::saveFile(currentDirPath + "/AbortSequence.json", jsonAbortSequence.dump(4));
    utils::saveFile(currentDirPath + "/comments.txt", comments);

    std::experimental::filesystem::copy("config_Franz.json", currentDirPath + "/");

}

void SequenceManager::WritePostSeqComment(std::string msg){
    utils::saveFile(lastDir + "/postseq-comments.txt", msg);
}

bool SequenceManager::LoadSequence(nlohmann::json jsonSeq)
{
    deviceMap.clear();
    sensorsNominalRangeMap.clear();
    sensorsNominalRangeTimeMap.clear();
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
                            sensorsNominalRangeMap[sensorsIt.key()][timestampMicros][0] = sensorsIt.value()[0];
                            sensorsNominalRangeMap[sensorsIt.key()][timestampMicros][1] = sensorsIt.value()[1];
                            sensorsNominalRangeTimeMap[timestampMicros][sensorsIt.key()][0] = sensorsIt.value()[0];
                            sensorsNominalRangeTimeMap[timestampMicros][sensorsIt.key()][1] = sensorsIt.value()[1];
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
    if (!isRunning && !isAbortRunning)
    {
        jsonSequence = jsonSeq;
        jsonAbortSequence = jsonAbortSeq;
        SequenceManager::comments = comments;

        if (LoadSequence(jsonSeq))
        {
            SetupLogging();

            //TODO: MP Set warning light here or insert into sequence

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
            for (auto rangeName : jsonSeq["globals"]["ranges"])
            {
                Debug::info("Sensor nominal range found: %s", ((std::string) rangeName).c_str());
                if (rangeName.type() == nlohmann::json::value_t::string)
                {
                    msg += (std::string) rangeName + "Min;";
                    msg += (std::string) rangeName + "Max;";
                }
                else
                {
                    Debug::error("range name in sequence globals not a string");
                }

            }
            for (auto &item : deviceMap)
            {
                msg += item.first + ";";
            }

            Debug::log(/*"Timestep;" +*/ msg + "\n");

            isRunning = true;

            LoadInterpolationMap();

            int64_t startTime = utils::toMicros(jsonSeq["globals"]["startTime"]);
            int64_t endTime = utils::toMicros(jsonSeq["globals"]["endTime"]);
            int64_t interval = utils::toMicros(jsonSeq["globals"]["interval"]);
            Debug::info("%d %d %d", startTime, endTime, interval);

            EcuiSocket::SendJson("timer-start");

            timer->start(startTime, endTime, interval, std::bind(&SequenceManager::Tick, this, std::placeholders::_1), 
                std::bind(&SequenceManager::StopSequence, this));

            Debug::print("Start Sequence");
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
    std::map<std::string, std::tuple<double, uint64_t>> sensors = llInterface->GetLatestSensorData();

    for (const auto& sensor : sensors)
    {
        if (isAutoAbort && (sensorsNominalRangeMap.find(sensor.first) != sensorsNominalRangeMap.end()))
        {
            auto currInterval = sensorsNominalRangeMap[sensor.first].begin();
            double currValue = std::get<0>(sensor.second);
            if (currInterval->second[0] > currValue)
            {
                std::stringstream stream;
                stream << std::fixed << "auto abort Sensor: " << sensor.first << " value " + std::to_string(currValue) << " too low" << " at Time " << std::setprecision(2) << ((microTime/1000)/1000.0) << " seconds";
                std::string abortMsg = stream.str();
                if (isRunning)
                {
                    AbortSequence(abortMsg);
                }
            }
            else if (std::get<0>(sensor.second) > currInterval->second[1])
            {
                std::stringstream stream;
                stream << std::fixed << "auto abort Sensor: " << sensor.first << " value " + std::to_string(currValue) << " too high" << " at Time " << std::setprecision(2) << ((microTime/1000)/1000.0) << " seconds";
                std::string abortMsg = stream.str();
                if (isRunning)
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
            }
        }
        else
        {
            time = obj["timestamp"];
        }
    }
    else
    {
        Debug::error("in GetTimestamp: timestamp key of object does not exist");
    }

    return time;
}

void SequenceManager::Tick(int64_t microTime)
{
    if (microTime % 500000 == 0)
    {
        Debug::info("Micro Seconds: %d", microTime);
    }
    if (microTime % timerSyncInterval == 0)
    {
        EcuiSocket::SendJson("timer-sync", ((microTime/1000) / 1000.0));
    }
    if (microTime == 0)
    {
        //TODO: MP Set warning light here or insert into sequence
    }

    std::chrono::time_point<std::chrono::high_resolution_clock> beforeLogging;
    if (isRunning)
    {
        std::string msg = std::to_string(microTime / 1000000.0) + ";";
        syncMtx.lock();
        if (isRunning)
        {
            //log nominal ranges
            for (const auto sensor : sensorsNominalRangeMap)
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

                    if (microTime >= nextIt->first)
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
                                nextValue[0] = (scale * (microTime - prevIt->first)) + prevIt->second[0];
                                break;
                            }
                            case Interpolation::NONE:
                            default:
                                nextValue = prevIt->second;
                                shallExec = false;
                        }
                    }

                    if (shallExec)
                    {
                        eventManager->ExecuteCommand(devItem.first, nextValue, false);
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
        }
        //delete depricated timestamp and update sensorsNominalRangeMap as well
        if (sensorsNominalRangeTimeMap.size() > 1)
        {
            auto beginRangeIt = sensorsNominalRangeTimeMap.begin();
            int64_t nextRangeChange = beginRangeIt->first;
            if (microTime >= nextRangeChange)
            {
                for (const auto& sensor : beginRangeIt->second)
                {
                    sensorsNominalRangeMap[sensor.first].erase(sensorsNominalRangeMap[sensor.first].begin());
                }
                sensorsNominalRangeTimeMap.erase(beginRangeIt);
                //plotMaps();
                Debug::info("updated sensor ranges at time: %d in ms", microTime/1000);
            }
        }

        CheckSensors(microTime);

        syncMtx.unlock();
        Debug::log(msg + "\n");
    }
}

void SequenceManager::StopAbortSequence()
{
    if (isAbortRunning)
    {
        Debug::flush();
        Debug::print("Abort Sequence Done");
        isAbortRunning = false;
    }
}

void SequenceManager::StartAbortSequence()
{
    if (!isRunning && !isAbortRunning)
    {
        isAbortRunning = true;

        syncMtx.lock();

        for (auto it = jsonAbortSequence["actions"].begin(); it != jsonAbortSequence["actions"].end(); ++it)
        {
            if (it.key().compare("timestamp") != 0)
            {
                Debug::info(it.key() + " | %d", (uint8_t)it.value());
                std::vector<double> valueList = it.value();
                eventManager->ExecuteCommand(it.key(), valueList, false);
            }
        }
        syncMtx.unlock();
        StopAbortSequence();
    }

}

bool SequenceManager::IsSequenceRunning()
{
    return isRunning || isAbortRunning;
}