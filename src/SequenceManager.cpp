//
// Created by Markus on 2019-09-27.
//
#include <iomanip>
#include <sstream>

#include "SequenceManager.h"
#include "json.hpp"
#include "EcuiSocket.h"

#include "utils.h"
#include "LLInterface.h"

//#include "spdlog/async.h"
//#include "spdlog/sinks/basic_file_sink.h"

using json = nlohmann::json;

using namespace std;

bool SequenceManager::isRunning = false;
bool SequenceManager::isAbort = false;
bool SequenceManager::isAbortRunning = false;
std::mutex SequenceManager::syncMtx;
Timer* SequenceManager::timer;
Timer* SequenceManager::sensorTimer;

int32 threadCounter = 0;
//std::shared_ptr<spdlog::logger> SequenceManager::async_file;

json SequenceManager::jsonSequence = json::object();
json SequenceManager::jsonAbortSequence = json::object();

std::map<std::string, Point[2]> SequenceManager::sequenceIntervalMap;
std::map<std::string, double[2]> SequenceManager::sensorsNominalRangeMap;

typedef std::chrono::high_resolution_clock Clock;

void SequenceManager::init()
{
    timer = new Timer();
    sensorTimer = new Timer();
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
        Debug::error("cannot abort sequence: no running sequence");
    }

}

void SequenceManager::ChangeLogFile()
{
    time_t curr_time;
    tm * curr_tm;
    char dateTime_string[100];

    time(&curr_time);
    curr_tm = localtime(&curr_time);

    strftime(dateTime_string, 100, "%Y_%m_%d__%H_%M_%S", curr_tm);
//    logging::configure({ {"type", "file"}, {"file_name", "logs/" + string(dateTime_string) + ".csv"}, {"reopen_interval", "1"} });
//    logging::get_logger().setFilePath("logs/" + string(dateTime_string) + ".csv");
    Debug::changeOutputFile("../tmpLogs/" + string(dateTime_string) + ".csv");
}

void SequenceManager::StartSequence(json jsonSeq, json jsonAbortSeq)
{
    if (!isRunning && !isAbortRunning)
    {
        ChangeLogFile();

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
        for (auto item : outputNames)
        {
            msg += item + ";";
        }

        //async_file->info("Timestep;" + msg);
        //logging::INFO("Timestep;" + msg + "\n");
        Debug::log("Timestep;" + msg + "\n");

        isRunning = true;
        jsonSequence = jsonSeq;
        jsonAbortSequence = jsonAbortSeq;

        LoadIntervalMap();

        threadCounter = 0;


        int64 startTime = utils::toMicros(jsonSeq["globals"]["startTime"]);
        int64 endTime = utils::toMicros(jsonSeq["globals"]["endTime"]);
        int64 interval = utils::toMicros(jsonSeq["globals"]["interval"]);
        Debug::info("%d %d %d", startTime, endTime, interval);

        EcuiSocket::SendJson("timer-start");

        LLInterface::EnableAllOutputDevices();
        sensorTimer->startContinous(startTime, 1000000/SENSOR_SAMPLE_RATE, SequenceManager::GetSensors, SequenceManager::StopGetSensors);
        timer->start(startTime, endTime, interval, SequenceManager::Tick, SequenceManager::StopSequence);
        //TODO: discuss if we need that feature
//        if (HcpManager::EnableAllServos())
//        {
//            timer->start(startTime, endTime, interval, Tick, StopSequence);
//        }
//        else
//        {
//            Debug::error("couldn't enable all servos, aborting...");
//            AbortSequence();
//        }

    }
}

void SequenceManager::LoadIntervalMap()
{
    for (auto dataItem : jsonSequence["data"])
    {
        for (auto actionItem : dataItem["actions"])
        {

            float time;
            if (actionItem["timestamp"].type() == json::value_t::string)
            {
                string timeStr = actionItem["timestamp"];
                if (timeStr.compare("START") == 0)
                {
                    time = utils::toMicros(jsonSequence["globals"]["startTime"]);

                    int64 timestampMicros = utils::toMicros(time);
                    for (auto it = actionItem.begin(); it != actionItem.end(); ++it)
                    {
                        if (it.key().compare("timestamp") != 0 && it.key().compare("sensorsNominalRange") != 0)
                        {
                            sequenceIntervalMap[it.key()][0].x = timestampMicros;
                            sequenceIntervalMap[it.key()][0].y = (uint8)it.value();
                            sequenceIntervalMap[it.key()][1].x = timestampMicros;
                            sequenceIntervalMap[it.key()][1].y = (uint8)it.value();

                            Debug::print("Interval created for " + it.key());
                            Debug::print("prev: x: %d, y: %d", sequenceIntervalMap[it.key()][0].x, sequenceIntervalMap[it.key()][0].y);
                            Debug::print("next: x: %d, y: %d", sequenceIntervalMap[it.key()][1].x, sequenceIntervalMap[it.key()][1].y);
                        }
                    }
                }
            }


        }
    }
}

void SequenceManager::UpdateIntervalMap(std::string name, int64 microTime, uint8 newValue)
{
    if (sequenceIntervalMap.find(name) != sequenceIntervalMap.end())
    {
        sequenceIntervalMap[name][0] = sequenceIntervalMap[name][1];

        Point newPt = {};
        newPt.x = microTime;
        newPt.y = newValue;

        sequenceIntervalMap[name][1] = newPt;
    }
    else
    {
        Debug::error("Cannot find name in sequenceIntervalMap, please define every device at the START timestamp");
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
    }
    else
    {
        msg += ";";
    }
    //async_file->info(to_string(microTime) + ";" + msg);
    //logging::INFO("\n" + to_string(secs) + ";" + msg);
    Debug::log("\n" + to_string(secs) + ";" + msg);
}

void SequenceManager::StopGetSensors()
{
//    async_file->flush();
    Debug::flush();
}

void SequenceManager::GetSensors(int64 microTime)
{
    auto startTime = Clock::now();
    map<string, double> sensors = LLInterface::GetAllSensors();

    vector<double> vals;
    for (const auto& sensor : sensors)
    {
        if (sensorsNominalRangeMap.find(sensor.first) != sensorsNominalRangeMap.end())
        {
            if (sensorsNominalRangeMap[sensor.first][0] > sensor.second)
            {
                std::stringstream stream;
                stream << std::fixed << "auto abort Sensor: " << sensor.first << " value " + to_string(sensor.second) << " too low" << " at Time " << std::setprecision(2) << ((microTime/1000)/1000.0) << " seconds";
                string abortMsg = stream.str();
                if (isRunning)
                {
                    //SequenceManager::AbortSequence(abortMsg);
                }
            }
            else if (sensor.second > sensorsNominalRangeMap[sensor.first][1])
            {
                std::stringstream stream;
                stream << std::fixed << "auto abort Sensor: " << sensor.first << " value " + to_string(sensor.second) << " too high" << " at Time " << std::setprecision(2) << ((microTime/1000)/1000.0) << " seconds";
                string abortMsg = stream.str();
                if (isRunning)
                {
                    //SequenceManager::AbortSequence(abortMsg);
                }
            }
        }
        vals.push_back(sensor.second);
    }

    LogSensors(microTime, vals);

    if (microTime % SENSOR_TRANSMISSION_INTERVAL == 0)
    {
//        std::thread callbackThread(SequenceManager::TransmitSensors, microTime, sensors);
//        callbackThread.detach();
	    LLInterface::TransmitSensors(microTime, sensors);
    }
    auto currTime = Clock::now();
    //cerr << "Transmission Timer elapsed: " << std::chrono::duration_cast<std::chrono::microseconds>(currTime-startTime).count() << endl;

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
    if (microTime % TIMER_SYNC_INTERVAL == 0)
    {
        EcuiSocket::SendJson("timer-sync", ((microTime/1000) / 1000.0));
    }
    if (microTime == 0)
    {
        LLInterface::TurnRed();
    }

    bool findNext = false;
    map<string, uint8> namesUpdated;


    //TODO: this is very inefficient right now; make it so actions can be accessed by timestamp as key
    for (auto dataItem : jsonSequence["data"])
    {
        for (auto actionItem : dataItem["actions"])
        {

            float time = 0.0;
            if (actionItem["timestamp"].type() == json::value_t::string)
            {
                string timeStr = actionItem["timestamp"];
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
                time = actionItem["timestamp"];
            }

            int64 timestampMicros = utils::toMicros(time);
            if (timestampMicros == microTime)
            {
                Debug::print("timestamp | %d", time);
                for (auto it = actionItem.begin(); it != actionItem.end(); ++it)
                {
                    if (it.key().compare("sensorsNominalRange") == 0)
                    {
                        json sensorsRanges = it.value();
                        for (auto sensorsIt = sensorsRanges.begin(); sensorsIt != sensorsRanges.end(); ++sensorsIt)
                        {
                            if (sensorsIt.value().type() == json::value_t::array && sensorsIt.value().size() == 2)
                            {
                                sensorsNominalRangeMap[sensorsIt.key()][0] = sensorsIt.value()[0];
                                sensorsNominalRangeMap[sensorsIt.key()][1] = sensorsIt.value()[1];
                            }
                            else
                            {
                                cout << "Range of " << sensorsIt.key() << " not valid" << endl;
                            }
                        }
                    }
                    else if (it.key().compare("timestamp") != 0)
                    {
                        findNext = true;
                        Debug::print(it.key() + " | %d", (uint8)it.value());
                        namesUpdated[it.key()] = (uint8)it.value();
                    }
                }
                Debug::print("--------------");
            }
            else if (timestampMicros > microTime)
            {
                if (findNext)
                {
                    for (auto it = actionItem.begin(); it != actionItem.end(); ++it)
                    {
                        if (namesUpdated.find(it.key()) != namesUpdated.end())
                        {
                            UpdateIntervalMap(it.key(), timestampMicros, (uint8)it.value());

                            namesUpdated.erase(it.key());
                        }
                    }
                    if (namesUpdated.empty())
                    {
                        //all values updated
                        findNext = false;
                        break;
                    }

                }
            }
        }
    }
    std::chrono::time_point<std::chrono::high_resolution_clock> beforeLogging;
    if (findNext)
    {
        //not all values found yet; microTimes > endTime;
        Debug::warning("findNext still true at micro time: %d", microTime);

    }
    else
    {
        if (isRunning)
        {
            string msg = to_string(microTime / 1000000.0) + ";";
            syncMtx.lock();
            if (isRunning)
            {

                for (const auto &seqItem : sequenceIntervalMap)
                {
                    Point prev = sequenceIntervalMap[seqItem.first][0];
                    Point next = sequenceIntervalMap[seqItem.first][1];

                    uint8 nextValue;
                    if (prev.x == microTime)
                    {
                        nextValue = (uint8) prev.y;
                    }
                    else
                    {
                        double scale = ((next.y - prev.y) * 1.0) / (next.x - prev.x);
                        nextValue = (scale * (microTime - prev.x)) + prev.y;
                    }

                    msg += to_string(nextValue) + ";";

                    LLInterface::ExecCommand(seqItem.first, nextValue);
                    if (threadCounter > 1)
                    {
                        Debug::warning("writing " + seqItem.first + " with value %d at micro time: %d", nextValue,
                                     microTime);
                    }
                }
            }
            syncMtx.unlock();
		    beforeLogging = Clock::now();
            Debug::log(msg);
        }
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
                usleep(afterSeqLogTime);
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
