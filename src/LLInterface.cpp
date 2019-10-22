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

void LLInterface::Init()
{
    HcpManager::init();
    warnLight = new WarnLight(0);
    sensorTimer = new Timer();
    i2cDevice = new I2C(I2C_DEVICE_ADDRESS, "thrust");

    //set to one shot mode channel 1
    i2cDevice->Write8(0x00);
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
    sensorNames.push_back(i2cDevice->GetName() + "1");
    sensorNames.push_back(i2cDevice->GetName() + "2");
    sensorNames.push_back(i2cDevice->GetName() + "3");
    return sensorNames;
}

std::map<std::string, int32> LLInterface::GetAllSensors()
{
    std::map<std::string, int32> sensors;
    sensors = HcpManager::GetAllSensors();

    i2cDevice->Write8(0x00);
    uint16 val = i2cDevice->Read16();
    int raw_adc = (val & 0x0FFF);
    if(raw_adc > 2047)
    {
        raw_adc -= 4095;
    }
    sensors[i2cDevice->GetName() + "1"] = raw_adc;

    i2cDevice->Write8(0x20);
    val = i2cDevice->Read16();
    raw_adc = (val & 0x0FFF);
    if(raw_adc > 2047)
    {
        raw_adc -= 4095;
    }
    sensors[i2cDevice->GetName() + "2"] = raw_adc;

    i2cDevice->Write8(0x40);
    val = i2cDevice->Read16();
    raw_adc = (val & 0x0FFF);
    if(raw_adc > 2047)
    {
        raw_adc -= 4095;
    }
    sensors[i2cDevice->GetName() + "3"] = raw_adc;
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

    if (sensors.find("igniter feedback") != sensors.end())
    {
        if (sensors["igniter feedback"] > 0)
        {
            TurnRed();
            warnlightStatus = 2;
        }
        else if (warnlightStatus == 2)
        {
            TurnYellow();
            warnlightStatus = 1;
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
