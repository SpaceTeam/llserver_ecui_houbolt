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
bool LLInterface::isYellow = true;
Timer* LLInterface::sensorTimer;

void LLInterface::Init()
{
    HcpManager::init();
    i2cDevice = new I2C(I2C_DEVICE_ADDRESS, "thrust");
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
    sensorNames.push_back(i2cDevice->GetName());
    return sensorNames;
}

std::map<std::string, int32> LLInterface::GetAllSensors()
{
    std::map<std::string, int32> sensors;
    sensors = HcpManager::GetAllSensors();
    sensors[i2cDevice->GetName()] = i2cDevice->ReadByte() - 128;
    return sensors;
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
    }
}

void LLInterface::StopGetSensors()
{
    isTransmittingSensors = false;
}

void LLInterface::GetSensors(int64 microTime)
{
    std::map<std::string, int32> sensors = GetAllSensors();

    TransmitSensors(microTime, sensors);

    if (sensors.find("fuelPressure") != sensors.end())
    {
        if (sensors["fuelPressure"] >= 5000 && !isYellow)
        {
            turnYellow();
            isYellow = true;
        }
        else if (sensors["fuelPressure"] < 5000 && isYellow)
        {
            turnGreen();
            isYellow = false;
        }
    }

}

void LLInterface::TransmitSensors(int64 microTime, std::map<std::string, int32> sensors)
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

void LLInterface::turnRed()
{
    warnLight->Error();
}

void LLInterface::turnGreen()
{
    warnLight->SafeOn();
}

void LLInterface::turnYellow()
{
    warnLight->NoConnection();
}
