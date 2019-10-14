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

std::map<std::string, Point[2]> SequenceManager::sequenceIntervalMap;
std::map<std::string, double[2]> SequenceManager::sensorsNominalRangeMap;

I2C* SequenceManager::i2cDevice;

void SequenceManager::init()
{
    timer = new Timer();
    sensorTimer = new Timer();

    i2cDevice = new I2C(I2C_DEVICE_ADDRESS);
}


void SequenceManager::StopSequence()
{
    Debug::info("sequence done");
    std::cout << "i'm called" << std::endl;
    HcpManager::DisableAllServos();
    isRunning = false;
    if (!isAbort)
    {
        std::cout << "hello timer" << std::endl;
        Socket::sendJson("timer-done");
    }
}

void SequenceManager::AbortSequence(std::string abortMsg)
{
    if (isRunning)
    {
        isAbort = true;
        timer->stop();
        sensorTimer->stop();
        Socket::sendJson("abort", abortMsg);
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
        logging::get_logger().setFilePath("logs/" + string(dateTime_string) + ".csv");

        //get sensor names
        vector<string> sensorNames = HcpManager::GetAllSensorNames();
        string msg;
        for (int i = 0; i < sensorNames.size(); i++)
        {
            msg += sensorNames[i] + ";";
        }
        //add thrust sensor
        msg += "thrust;";

        //async_file->info("Timestep;" + msg);
        logging::INFO("Timestep;" + msg);

        isRunning = true;
        jsonSequence = jsonSeq;
        jsonAbortSequence = jsonAbortSeq;

        LoadIntervalMap();

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

    //add thrust sensor value
    sensors["thrust"] = i2cDevice->ReadByte();

    vector<uint16> vals;
    for (const auto& sensor : sensors)
    {
        if (sensorsNominalRangeMap.find(sensor.first) != sensorsNominalRangeMap.end())
        {
            if (sensorsNominalRangeMap[sensor.first][0] > sensor.second || sensor.second > sensorsNominalRangeMap[sensor.first][1])
            {
                string abortMsg = "auto abort Sensor: " + sensor.first + " FATAL value : " + to_string(sensor.second);
                SequenceManager::AbortSequence(abortMsg);
            }
        }
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

    bool findNext = false;
    map<string, uint8> namesUpdated;


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

    if (findNext)
    {
        //not all values found yet; microTimes > endTime;

    }
    else
    {
        for (const auto& seqItem : sequenceIntervalMap)
        {
            Point prev = sequenceIntervalMap[seqItem.first][0];
            Point next = sequenceIntervalMap[seqItem.first][1];

            uint8 nextValue;
            if (prev.x == microTime)
            {
                nextValue = (uint8)prev.y;
            }
            else
            {
                double scale = ((next.y - prev.y)*1.0) / (next.x - prev.x);
                nextValue = (scale * (microTime-prev.x)) + prev.y;
            }
            Debug::info("writing " + seqItem.first + " with value %d", nextValue);

            HcpManager::ExecCommand(seqItem.first, nextValue);
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
