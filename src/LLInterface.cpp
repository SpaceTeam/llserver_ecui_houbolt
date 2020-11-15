//
// Created by Markus on 2019-10-15.
//

#include "HcpManager.h"
#include "EcuiSocket.h"
#include "Timer.h"
#include "Config.h"

#include "LLInterface.h"

I2C* LLInterface::i2cDevice;
WarnLight* LLInterface::warnLight;
TMPoE *LLInterface::tmPoE;

//GPIO[] LLInterface::gpioDevices;

//SPI* LLInterface::spiDevice;

bool LLInterface::isInitialized = false;

bool LLInterface::useTMPoE = false;

bool LLInterface::isTransmittingSensors = false;
int32 LLInterface::warnlightStatus = true;
Timer* LLInterface::sensorTimer;

typedef std::chrono::high_resolution_clock Clock;

uint32 threadCount2 = 0;

void LLInterface::Init()
{
    if (!isInitialized)
    {
        HcpManager::Init();
        HcpManager::StartSensorFetch(std::get<int>(Config::getData("HCP/sensor_sample_rate")));
        sensorTimer = new Timer(40, "sensorTimer");
        warnLight = new WarnLight(0);

        useTMPoE = std::get<bool>(Config::getData("useTMPoE"));
        if (useTMPoE)
        {
            tmPoE = new TMPoE(0, 50);
        }
        //i2cDevice = new I2C(0, "someDev"); //not in use right now

        isInitialized = true;
    }
}

void LLInterface::Destroy()
{
    if (isInitialized)
    {
        //delete i2cDevice;
        delete warnLight;
        if (useTMPoE)
        {
            delete tmPoE;
        }
        HcpManager::StopSensorFetch();
        HcpManager::Destroy();
    }
}

void LLInterface::EnableAllOutputDevices()
{
    HcpManager::EnableAllServos();
}

void LLInterface::DisableAllOutputDevices()
{
    HcpManager::DisableAllServos();
}

std::vector<std::string> LLInterface::GetAllSensorNames()
{
    std::vector<std::string> sensorNames;
    sensorNames = HcpManager::GetAllSensorNames();

    if (useTMPoE)
    {
        for (int i = 0; i < 8; i++)
        {
            sensorNames.push_back("temp " + std::to_string(i+1));
        }
    }

    std::sort(sensorNames.begin(), sensorNames.end());
    return sensorNames;
}

std::map<std::string, double> LLInterface::GetAllSensors()
{
    //auto startTime = Clock::now();
    std::map<std::string, double> sensors;
    sensors = HcpManager::GetAllSensors();

    if (useTMPoE)
    {
        std::vector<uint32> tmpValues = tmPoE->Read();
        for (int i = 0; i < tmpValues.size(); i++)
        {
            sensors["temp " + std::to_string(i+1)] = tmpValues[i];
        }
    }

    //auto currTime = Clock::now();
    //std::cerr << "Get Sensors Timer elapsed: " << std::chrono::duration_cast<std::chrono::microseconds>(currTime-startTime).count() << std::endl;

    return sensors;
}

std::vector<std::string> LLInterface::GetAllOutputNames()
{
    std::vector<std::string> outputNames;
    outputNames = HcpManager::GetAllOutputNames();
    return outputNames;
}

nlohmann::json LLInterface::GetAllServoData()
{
    return HcpManager::GetAllServoData();
}

bool LLInterface::ExecCommand(std::string name, json value)
{
    if (value.type() != json::value_t::number_float &&
        value.type() != json::value_t::number_integer &&
        value.type() != json::value_t::number_unsigned)
    {
        return false;
    }
    return HcpManager::ExecCommand(name, (uint8)value);
}

void LLInterface::StartSensorTransmission()
{
    if (!isTransmittingSensors)
    {
        isTransmittingSensors = true;
        sensorTimer->startContinous(0,100000, LLInterface::GetSensors, LLInterface::StopGetSensors);
    }
}

void LLInterface::StopSensorTransmission()
{
    if (isTransmittingSensors)
    {

        sensorTimer->stop();
        isTransmittingSensors = false;
    }
}

void LLInterface::StopGetSensors()
{

}

void LLInterface::GetSensors(int64 microTime)
{
    // struct timespec ts;
    // clock_gettime(CLOCK_MONOTONIC, &ts);
    // printf("ts: %d %09d\n",ts.tv_sec, ts.tv_nsec);

    threadCount2++;
    if (threadCount2 > 1)
    {
        Debug::error("Transmitting Sensor Threads running: %d", threadCount2);
    }

    std::map<std::string, double> sensors = GetAllSensors();

    TransmitSensors(microTime, sensors);

    if (sensors.find("igniter feedback") != sensors.end())
    {
        if (sensors["igniter feedback"] == 0 && warnlightStatus != 2)
        {
            TurnRed();
            warnlightStatus = 2;
        }
    }
    if (sensors.find("fuelPressure") != sensors.end())
    {
        if ((sensors["fuelPressure"] >= 5000 || sensors["oxidizerPressure"] >= 5000) && warnlightStatus==0)
        {
            TurnYellow();
            warnlightStatus = 1;
        }
        else if ((sensors["fuelPressure"] < 5000 && sensors["oxidizerPressure"] < 5000) && warnlightStatus==1)
        {
            TurnGreen();
            warnlightStatus = 0;
        }
    }

    threadCount2--;

}

void LLInterface::TransmitSensors(int64 microTime, std::map<std::string, double> sensors)
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
    EcuiSocket::SendJson("sensors", content);
}

void LLInterface::TurnRed()
{
    warnLight->SetColor(255, 0, 0);
    warnLight->SetMode("default");
    warnLight->StopBuzzer();
}

void LLInterface::TurnGreen()
{
    warnLight->SetColor(0, 255, 0);
    warnLight->SetMode("default");
    warnLight->StopBuzzer();
}

void LLInterface::TurnYellow()
{
    warnLight->SetColor(0, 255, 255);
    warnLight->SetMode("spin");
    warnLight->StopBuzzer();
}

void LLInterface::BeepRed()
{
    warnLight->SetColor(255, 0, 0);
    warnLight->SetMode("blink");
    warnLight->StartBuzzerBeep(500);
}
