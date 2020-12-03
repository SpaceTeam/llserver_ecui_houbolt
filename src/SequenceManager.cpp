//
// Created by Markus on 2019-09-27.
//
#include <iomanip>
#include <sstream>
#include <filesystem>

#include "SequenceManager.h"
#include "json.hpp"
#include "EcuiSocket.h"

#include "utils.h"
#include "LLInterface.h"
#include "Config.h"

//#include "spdlog/async.h"
//#include "spdlog/sinks/basic_file_sink.h"

using json = nlohmann::json;

using namespace std;

bool SequenceManager::isRunning = false;
bool SequenceManager::isAutoAbort = true;
bool SequenceManager::isAbort = false;
bool SequenceManager::isAbortRunning = false;

int32 SequenceManager::sensorTransmissionInterval = 0;
int32 SequenceManager::sensorSampleRate = 0;
int32 SequenceManager::timerSyncInterval = 0;

std::mutex SequenceManager::syncMtx;
Timer* SequenceManager::timer;
Timer* SequenceManager::sensorTimer;

int32 threadCounter = 0;

json SequenceManager::jsonSequence = json::object();
json SequenceManager::jsonAbortSequence = json::object();
string SequenceManager::comments = "";
string SequenceManager::currentDirPath = "";
string SequenceManager::logFileName = "";
string SequenceManager::lastDir = "";

std::map<std::string, Interpolation> SequenceManager::interpolationMap;
std::map<int64, std::map<std::string, double[2]>> SequenceManager::sensorsNominalRangeTimeMap;
std::map<std::string, std::map<int64, double[2]>> SequenceManager::sensorsNominalRangeMap;
std::map<std::string, std::map<int64, double>> SequenceManager::deviceMap;

typedef std::chrono::high_resolution_clock Clock;

void SequenceManager::plotMaps(uint8 option=2)
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
                Debug::info("\t%d: %.2f", itemTimestamp.first, itemTimestamp.second);
            }

        }
    }

}

void SequenceManager::init()
{
    timer = new Timer();
    sensorTimer = new Timer();

    isAutoAbort = std::get<bool>(Config::getData("autoabort"));

    sensorTransmissionInterval = std::get<int>(Config::getData("WEBSERVER/sensor_transmission_interval"));
    sensorSampleRate = std::get<int>(Config::getData("LOGGING/sensor_log_rate"));
    timerSyncInterval = std::get<int>(Config::getData("WEBSERVER/timer_sync_interval"));
}


void SequenceManager::StopSequence()
{
    Debug::flush();
    Debug::info("sequence done");
    LLInterface::TurnYellow();
    isRunning = false;
    if (!isAbort)
    {
        sensorTimer->stop();
        LLInterface::DisableAllOutputDevices();
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
        Debug::print("aborting... " + abortMsg);
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

    currentDirPath = "logs/" + string(dateTime_string);
    SequenceManager::lastDir = currentDirPath;
    logFileName = string(dateTime_string) + ".csv";
    filesystem::create_directory(currentDirPath);
    Debug::changeOutputFile(currentDirPath + "/" + string(dateTime_string) + ".csv");

    //save Sequence files
    utils::saveFile(currentDirPath + "/Sequence.json", jsonSequence.dump(4));
    utils::saveFile(currentDirPath + "/AbortSequence.json", jsonAbortSequence.dump(4));
    utils::saveFile(currentDirPath + "/comments.txt", comments);

    filesystem::copy("config.json", currentDirPath + "/");
}

void SequenceManager::WritePostSeqComment(std::string msg){
    utils::saveFile(lastDir + "/postseq-comments.txt", msg);
}

bool SequenceManager::LoadSequence(json jsonSeq)
{
    deviceMap.clear();
    sensorsNominalRangeMap.clear();
    sensorsNominalRangeTimeMap.clear();
    for (auto dataItem : jsonSeq["data"])
    {
        double timeCmd = GetTimestamp(dataItem);
        int64 timestampCmdMicros = utils::toMicros(timeCmd);

        for (auto actionItem : dataItem["actions"])
        {
            //convert timestamp of action
            if (actionItem["timestamp"].type() == json::value_t::string)
            {
                Debug::error("no strings in actionitems allowed");
                SequenceManager::AbortSequence("no strings as timestamp in action items allowed");
                return false;
            }

            double time = GetTimestamp(actionItem);
            int64 timestampMicros = utils::toMicros(time);
            if (timestampMicros < 0)
            {
                Debug::error("timestamp in action must be positive");
                SequenceManager::AbortSequence("timestamp in action must be positive");
                return false;
            }

            timestampMicros += timestampCmdMicros;

            actionItem.erase("timestamp");
            for (auto it = actionItem.begin(); it != actionItem.end(); ++it)
            {
                if (it.key().compare("sensorsNominalRange") == 0)
                {
                    json sensorsRanges = it.value();
                    for (auto sensorsIt = sensorsRanges.begin(); sensorsIt != sensorsRanges.end(); ++sensorsIt)
                    {
                        if (sensorsIt.value().type() == json::value_t::array && sensorsIt.value().size() == 2)
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
                    deviceMap[it.key()][timestampMicros] = it.value();
                }
            }

        }
    }
    plotMaps();
    return true;
}

void SequenceManager::StartSequence(json jsonSeq, json jsonAbortSeq, std::string comments)
{
    if (!isRunning && !isAbortRunning)
    {
        jsonSequence = jsonSeq;
        jsonAbortSequence = jsonAbortSeq;
        SequenceManager::comments = comments;

        if (LoadSequence(jsonSeq))
        {
            SetupLogging();

            LLInterface::BeepRed();

            //get sensor names
            vector<string> sensorNames = LLInterface::GetAllSensorNames();
            vector<string> outputNames = LLInterface::GetAllOutputNames();
            string msg;
            for (int i = 0; i < sensorNames.size(); i++)
            {
                msg += sensorNames[i] + ";";
            }
            msg += "Status;";
            msg += "SequenceTime;";
            for (auto rangeName : jsonSeq["globals"]["ranges"])
            {
                cerr << rangeName << endl;
                if (rangeName.type() == json::value_t::string)
                {
                    msg += (string) rangeName + "Min;";
                    msg += (string) rangeName + "Max;";
                }
                else
                {
                    Debug::error("range name in sequence globals not a string");
                }

            }
            for (auto item : outputNames)
            {
                msg += item + ";";
            }

            Debug::log("Timestep;" + msg + "\n");

            isRunning = true;

            LoadInterpolationMap();

            threadCounter = 0;


            int64 startTime = utils::toMicros(jsonSeq["globals"]["startTime"]);
            int64 endTime = utils::toMicros(jsonSeq["globals"]["endTime"]);
            int64 interval = utils::toMicros(jsonSeq["globals"]["interval"]);
            Debug::info("%d %d %d", startTime, endTime, interval);

            EcuiSocket::SendJson("timer-start");

            LLInterface::EnableAllOutputDevices();
            sensorTimer->startContinous(startTime, 1000000 / sensorSampleRate, SequenceManager::GetSensors,
                                        SequenceManager::StopGetSensors);
            timer->start(startTime, endTime, interval, SequenceManager::Tick, SequenceManager::StopSequence);
        }

    }
}

void SequenceManager::LoadInterpolationMap()
{
    for (auto it = jsonSequence["globals"]["interpolation"].begin(); it != jsonSequence["globals"]["interpolation"].end(); ++it)
    {
        string mode = it.value();
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

void SequenceManager::LogSensors(int64 microTime, vector<double> sensors)
{
    string msg;
    double secs = microTime/1000000.0;
    for (int i = 0; i < sensors.size(); i++)
    {
        msg += to_string(sensors[i]) + ";";
    }
    if (!isRunning)
    {
        msg += "Auto Abort;";

        //TODO: think about a better solution to log on abort
        msg += to_string(secs) + ";";
        for (const auto& sensor : sensorsNominalRangeMap)
        {
            msg += to_string(sensor.second.begin()->second[0]) + ";";
            msg += to_string(sensor.second.begin()->second[1]) + ";";
        }
        for (auto it = jsonAbortSequence["actions"].begin(); it != jsonAbortSequence["actions"].end(); ++it)
        {
            msg += to_string(it.value()) + ";";
        }
    }
    else
    {
        msg += ";";
    }
    Debug::log("\n" + to_string(secs) + ";" + msg);


}

void SequenceManager::StopGetSensors()
{
    Debug::flush();
    // execute gnuplot script
    std::string scriptPath = std::get<std::string>(Config::getData("LOGGING/post_sequence_script"));
    system((scriptPath + " " + currentDirPath + " " + logFileName).c_str());
}

void SequenceManager::GetSensors(int64 microTime)
{
    map<string, double> sensors = LLInterface::GetAllSensors();

    vector<double> vals;
    for (const auto& sensor : sensors)
    {
        if (isAutoAbort && (sensorsNominalRangeMap.find(sensor.first) != sensorsNominalRangeMap.end()))
        {
            auto currInterval = sensorsNominalRangeMap[sensor.first].begin();
            if (currInterval->second[0] > sensor.second)
            {
                std::stringstream stream;
                stream << std::fixed << "auto abort Sensor: " << sensor.first << " value " + to_string(sensor.second) << " too low" << " at Time " << std::setprecision(2) << ((microTime/1000)/1000.0) << " seconds";
                string abortMsg = stream.str();
                if (isRunning)
                {
                    SequenceManager::AbortSequence(abortMsg);
                }
            }
            else if (sensor.second > currInterval->second[1])
            {
                std::stringstream stream;
                stream << std::fixed << "auto abort Sensor: " << sensor.first << " value " + to_string(sensor.second) << " too high" << " at Time " << std::setprecision(2) << ((microTime/1000)/1000.0) << " seconds";
                string abortMsg = stream.str();
                if (isRunning)
                {
                    SequenceManager::AbortSequence(abortMsg);
                }
            }
        }
        vals.push_back(sensor.second);
    }

    LogSensors(microTime, vals);

    if (microTime % sensorTransmissionInterval == 0)
    {
//        std::thread callbackThread(SequenceManager::TransmitSensors, microTime, sensors);
//        callbackThread.detach();
	    LLInterface::TransmitSensors(microTime, sensors);
    }

}

double SequenceManager::GetTimestamp(json obj)
{
    //convert timestamp of action
    double time = 0.0;
    if (utils::keyExists(obj, "timestamp"))
    {
        if (obj["timestamp"].type() == json::value_t::string)
        {
            string timeStr = obj["timestamp"];
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

void SequenceManager::Tick(int64 microTime)
{
    threadCounter++;
    auto startTime = Clock::now();

    if (threadCounter > 1)
    {
        cerr << "Threads: " << threadCounter << " micros: " << microTime << endl;
    }
    if (threadCounter > 90)
    {
        cerr << "too many threads, pausing" << endl;
        threadCounter--;
        return;
    }

    if (microTime % 500000 == 0)
    {
        Debug::print("Micro Seconds: %d", microTime);
    }
    if (microTime % timerSyncInterval == 0)
    {
        EcuiSocket::SendJson("timer-sync", ((microTime/1000) / 1000.0));
    }
    if (microTime == 0)
    {
        LLInterface::TurnRed();
    }

    std::chrono::time_point<std::chrono::high_resolution_clock> beforeLogging;
    if (isRunning)
    {
        string msg = to_string(microTime / 1000000.0) + ";";
        syncMtx.lock();
        if (isRunning)
        {
            //log nominal ranges
            for (const auto sensor : sensorsNominalRangeMap)
            {
                msg += to_string(sensor.second.begin()->second[0]) + ";";
                msg += to_string(sensor.second.begin()->second[1]) + ";";
            }

            bool shallExec;
            uint8 nextValue = 0;

            for (const auto &devItem : deviceMap)
            {
                shallExec = true;

                if (devItem.second.size() > 1)
                {
                    auto prevIt = devItem.second.begin();
                    auto nextIt = std::next(devItem.second.begin());

                    if (microTime >= nextIt->first)
                    {
                        nextValue = (uint8) nextIt->second;
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
                                double scale = ((nextIt->second - prevIt->second) * 1.0) / (nextIt->first - prevIt->first);
                                nextValue = (scale * (microTime - prevIt->first)) + prevIt->second;
                                break;
                            }
                            case Interpolation::NONE:
                            default:
                                nextValue = (uint8) prevIt->second;
                                shallExec = false;
                        }
                    }

                    if (shallExec)
                    {
                        LLInterface::ExecCommand(devItem.first, nextValue);
                    }
                    if (threadCounter > 1)
                    {
                        Debug::warning("writing " + devItem.first + " with value %d at micro time: %ld", nextValue,
                                     microTime);
                    }
                }
                else
                {
                    Debug::info("no interval found, keeping the same value");
                }

                msg += to_string(nextValue) + ";";
            }
        }
        //delete depricated timestamp and update sensorsNominalRangeMap as well
        if (sensorsNominalRangeTimeMap.size() > 1)
        {
            auto beginRangeIt = sensorsNominalRangeTimeMap.begin();
            int64 nextRangeChange = beginRangeIt->first;
            if (microTime >= nextRangeChange)
            {
                for (const auto& sensor : beginRangeIt->second)
                {
                    sensorsNominalRangeMap[sensor.first].erase(sensorsNominalRangeMap[sensor.first].begin());
                }
                sensorsNominalRangeTimeMap.erase(beginRangeIt);
                //plotMaps();
                Debug::error("updated sensor ranges at time: %d in ms", microTime/1000);
            }
        }

        syncMtx.unlock();
        beforeLogging = Clock::now();
        Debug::log(msg);
    }

    auto currTime = Clock::now();
    if (threadCounter > 80)
    {
        cerr << "Timer elapsed: " << std::chrono::duration_cast<std::chrono::microseconds>(currTime-startTime).count() <<  endl;
        cerr << "Time elapsed until logging" << std::chrono::duration_cast<std::chrono::microseconds>(beforeLogging-startTime).count() << endl;
        cerr << "Time elapsed during logging" << std::chrono::duration_cast<std::chrono::microseconds>(currTime-beforeLogging).count() << endl;
    }
    threadCounter--;
}

void SequenceManager::StopAbortSequence()
{
    if (isAbortRunning)
    {
        Debug::flush();
        Debug::info("abort sequence done");
        isAbortRunning = false;

        if (utils::keyExists(jsonAbortSequence["globals"], "endTime") &&
            (jsonAbortSequence["globals"]["endTime"].type() == json::value_t::number_float))
        {
            int64 afterSeqLogTime = ((double)jsonAbortSequence["globals"]["endTime"]) * 1000000.0;
            afterSeqLogTime = abs(afterSeqLogTime);

            printf("After Seq %lld \n", afterSeqLogTime);

            std::thread afterSeqLogThread([afterSeqLogTime](){
                //subtracted number used from sleep in start abort sequence
                usleep(afterSeqLogTime-50000);
                sensorTimer->stop();
            });
            afterSeqLogThread.detach();
        }
        else
        {
            sensorTimer->stop();
        }

        //wait some time so servos can move
        std::thread disableThread([](){
		    sleep(3);
       		LLInterface::DisableAllOutputDevices();
        });
        disableThread.detach();
    }
}

void SequenceManager::StartAbortSequence()
{
    if (!isRunning && !isAbortRunning)
    {
        isAbortRunning = true;
        usleep(50000);
        syncMtx.lock();

        for (auto it = jsonAbortSequence["actions"].begin(); it != jsonAbortSequence["actions"].end(); ++it)
        {
            if (it.key().compare("timestamp") != 0)
            {
                Debug::print(it.key() + " | %d", (uint8)it.value());
                LLInterface::ExecCommand(it.key(), it.value());
            }
        }
        syncMtx.unlock();
        Debug::print("--------------");
        StopAbortSequence();
    }

}
