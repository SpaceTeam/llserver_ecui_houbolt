//
// Created by Markus on 2019-10-15.
//

#include "HcpManager.h"

#include "LLInterface.h"

I2C* LLInterface::i2cDevice;
WarnLight* LLInterface::warnLight;
    //GPIO[] LLInterface::gpioDevices;

    //SPI* LLInterface::spiDevice;

void LLInterface::Init()
{
    HcpManager::init();
    i2cDevice = new I2C(I2C_DEVICE_ADDRESS, "thrust");
    warnLight = new WarnLight(0);

    warnLight->Error();
    warnLight->ServoCal();
    warnLight->NoConnection();
    warnLight->SafeOn();
    warnLight->SafeOff();
    warnLight->Standby();
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

std::map<std::string, uint16> LLInterface::GetAllSensors()
{
    std::map<std::string, uint16> sensors;
    sensors = HcpManager::GetAllSensors();
    sensors[i2cDevice->GetName()] = i2cDevice->ReadByte();
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
