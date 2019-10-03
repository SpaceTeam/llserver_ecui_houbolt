//
// Created by Markus on 2019-09-27.
//

#include "SequenceManager.h"
#include "json.hpp"
#include "Socket.h"

#include "utils.h"
#include "HcpManager.h"

//#include "spdlog/async.h"
//#include "spdlog/sinks/basic_file_sink.h"

using json = nlohmann::json;

using namespace std;

bool SequenceManager::isRunning = false;
bool SequenceManager::isAbort = false;
bool SequenceManager::isAbortRunning = false;
Timer* SequenceManager::timer;
Timer* SequenceManager::sensorTimer;

//std::shared_ptr<spdlog::logger> SequenceManager::async_file;

json SequenceManager::jsonSequence = json::object();
json SequenceManager::jsonAbortSequence = json::object();

void SequenceManager::init()
{
    timer = new Timer();
    sensorTimer = new Timer();
}


void SequenceManager::StopSequence()
{
    Debug::info("sequence done");
    std::cout << "i'm called" << std::endl;
    isRunning = false;
    if (!isAbort)
    {
        std::cout << "hello timer" << std::endl;
        Socket::sendJson("timer-done");
    }
}

void SequenceManager::AbortSequence()
{
    if (isRunning)
    {
        isAbort = true;
        timer->stop();
        sensorTimer->stop();
        Socket::sendJson("abort");
        Debug::print("aborting...");
        isRunning = false;
        StartAbortSequence();
        isAbort = false;
    }
    else
    {
        Debug::error("cannot abort sequence: no running sequence");
    }

}

void SequenceManager::StartSequence(json jsonSeq, json jsonAbortSeq)
{
    if (!isRunning && !isAbortRunning)
    {
        time_t curr_time;
        tm * curr_tm;
        char dateTime_string[100];

        time(&curr_time);
        curr_tm = localtime(&curr_time);

        strftime(dateTime_string, 100, "%d_%m_%Y__%H_%M_%S", curr_tm);
//        async_file = spdlog::basic_logger_mt<spdlog::async_factory>("async_file_logger", "logs/" + string(dateTime_string) + ".csv");
        logging::configure({ {"type", "file"}, {"file_name", "logs/" + string(dateTime_string) + ".csv"}, {"reopen_interval", "1"} });

        //get sensor names
        vector<string> sensorNames = HcpManager::GetAllSensorNames();
        string msg;
        for (int i = 0; i < sensorNames.size(); i++)
        {
            msg += sensorNames[i] + ";";
        }
        //async_file->info("Timestep;" + msg);
        logging::INFO("Timestep;" + msg);

        isRunning = true;
        jsonSequence = jsonSeq;
        jsonAbortSequence = jsonAbortSeq;


        int64 startTime = utils::toMicros(jsonSeq["globals"]["startTime"]);
        int64 endTime = utils::toMicros(jsonSeq["globals"]["endTime"]);
        int64 interval = utils::toMicros(jsonSeq["globals"]["interval"]);
        Debug::info("%d %d %d", startTime, endTime, interval);

        Socket::sendJson("timer-start");

        HcpManager::EnableAllServos();
        sensorTimer->start(startTime, endTime+3, 1000000/SENSOR_SAMPLE_RATE, SequenceManager::GetSensors, SequenceManager::StopGetSensors);
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

void SequenceManager::TransmitSensors(int64 microTime, std::map<std::string, uint16> sensors)
{
    json content = json::array();
    json sen;
    for (const auto& sensor : sensors)
    {
        sen = json::object();

        sen["name"] = sensor.first;
        sen["value"] = sensor.second;
        sen["time"] = (double)((microTime / 1000) / 1000.0);

        content.push_back(sen);
    }
    Socket::sendJson("sensors", content);
}

void SequenceManager::LogSensors(int64 microTime, vector<uint16> sensors)
{
    string msg;
    for (int i = 0; i < sensors.size(); i++)
    {
        msg += to_string(sensors[i]) + ";";
    }
    //async_file->info(to_string(microTime) + ";" + msg);
    logging::INFO(to_string(microTime) + ";" + msg);
}

void SequenceManager::StopGetSensors()
{
//    async_file->flush();
}

void SequenceManager::GetSensors(int64 microTime)
{
    map<string, uint16> sensors = HcpManager::GetAllSensors();

    vector<uint16> vals;
    for (const auto& sensor : sensors)
    {
        vals.push_back(sensor.second);
    }
    LogSensors(microTime, vals);

    if (microTime % SENSOR_TRANSMISSION_INTERVAL == 0)
    {
//        std::thread callbackThread(SequenceManager::TransmitSensors, microTime, sensors);
//        callbackThread.detach();
	TransmitSensors(microTime, sensors);
    }

}

void SequenceManager::Tick(int64 microTime)
{
    if (microTime % 500000 == 0)
    {
        Debug::print("Micro Seconds: %d", microTime);
    }
    if (microTime % TIMER_SYNC_INTERVAL == 0)
    {
        Socket::sendJson("timer-sync", ((microTime/1000) / 1000.0));
    }

    //TODO: this is very inefficient right now; make it so actions can be accessed by timestamp as key
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
                }
                else if (timeStr.compare("END") == 0)
                {
                    time = utils::toMicros(jsonSequence["globals"]["endTime"]);
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
                    if (it.key().compare("timestamp") != 0)
                    {
                        Debug::print(it.key() + " | %d", (uint8)it.value());
                        HcpManager::ExecCommand(it.key(), it.value());
                    }
                }
                Debug::print("--------------");
            }
            else if (timestampMicros > microTime)
            {
                break;
            }
        }
    }

}

void SequenceManager::StopAbortSequence()
{
    if (isAbortRunning)
    {
        Debug::info("abort sequence done");
        isAbortRunning = false;
        HcpManager::DisableAllServos();
    }
}

void SequenceManager::StartAbortSequence()
{
    if (!isRunning && !isAbortRunning)
    {
        isAbortRunning = true;

        for (auto it = jsonAbortSequence["actions"].begin(); it != jsonAbortSequence["actions"].end(); ++it)
        {
            if (it.key().compare("timestamp") != 0)
            {
                Debug::print(it.key() + " | %d", (uint8)it.value());
                HcpManager::ExecCommand(it.key(), it.value());
            }
        }
        Debug::print("--------------");
        StopAbortSequence();
    }

}
