//
// Created by Markus on 2019-10-15.
//

#include "HcpManager.h"
#include "EcuiSocket.h"
#include "Timer.h"

#include "LLInterface.h"

I2C* LLInterface::i2cDevice;
WarnLight* LLInterface::warnLight;
    //GPIO[] LLInterface::gpioDevices;

    //SPI* LLInterface::spiDevice;

bool LLInterface::isTransmittingSensors = false;
int32 LLInterface::warnlightStatus = true;
Timer* LLInterface::sensorTimer;

typedef std::chrono::high_resolution_clock Clock;

void LLInterface::Init()
{
    HcpManager::init();
    warnLight = new WarnLight(0);
    sensorTimer = new Timer();
}

void LLInterface::Destroy()
{
    delete i2cDevice;
    delete warnLight;
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
    return sensorNames;
}

std::map<std::string, double> LLInterface::GetAllSensors()
{
    auto startTime = Clock::now();
    std::map<std::string, double> sensors;
    sensors = HcpManager::GetAllSensors();

    auto currTime = Clock::now();
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
        sensorTimer->startContinous(100000, LLInterface::GetSensors, LLInterface::StopGetSensors);
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
    warnLight->Error();
}

void LLInterface::TurnGreen()
{
    warnLight->SafeOn();
}

void LLInterface::TurnYellow()
{
    warnLight->NoConnection();
}

void LLInterface::BeepRed()
{
    warnLight->Testing();
}
