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
int32 LLInterface::warnlightStatus = -1;
Timer* LLInterface::sensorTimer;

double LLInterface::sensorsSmoothingFactor = 0.0;
std::map<std::string, double> LLInterface::filteredSensorBuffer;

void LLInterface::Init()
{
    if (!isInitialized)
    {
        Debug::print("Initializing HcpManager...");
        HcpManager::Init();
        Debug::print("Initializing HcpManager done\n");
        Debug::print("Starting periodic sensor fetching...");
        HcpManager::StartSensorFetch(std::get<int>(Config::getData("HCP/sensor_sample_rate")));
        Debug::print("Periodic sensor fetching started\n");
        sensorTimer = new Timer(40, "SensorTimer");
        Debug::print("Connecting to warning light...");
        warnLight = new WarnLight(0);

        useTMPoE = std::get<bool>(Config::getData("useTMPoE"));
        if (useTMPoE)
        {
            Debug::print("Initializing TMPoE...");
            tmPoE = new TMPoE(0, 50);
        }
        //i2cDevice = new I2C(0, "someDev"); //not in use right now

		sensorsSmoothingFactor = std::get<double>(Config::getData("WEBSERVER/sensors_smoothing_factor"));

        isInitialized = true;
		//update warninglight after initialization but wait 1 sec to 
		//guarantee all sensors have already been fetched
		std::thread updateWarnlightThread([](){
					usleep(1000000);
			UpdateWarningLight();
				});
				updateWarnlightThread.detach();
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
        for (uint32 i = 0; i < tmpValues.size(); i++)
        {
            sensors["temp " + std::to_string(i+1)] = tmpValues[i];
        }
    }

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

nlohmann::json LLInterface::GetSupercharge()
{
    return HcpManager::GetSupercharge();
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
    std::map<std::string, double> sensors = GetAllSensors();

    TransmitSensors(microTime, sensors);

    UpdateWarningLight(sensors);

}

void LLInterface::FilterSensors(std::map<std::string, double> rawSensors)
{
	for (const auto& sensor : rawSensors)
    {
		if (filteredSensorBuffer.find(sensor.first) != filteredSensorBuffer.end())
		{
			filteredSensorBuffer[sensor.first] = sensor.second;
		}
		filteredSensorBuffer[sensor.first] += sensorsSmoothingFactor * (sensor.second - filteredSensorBuffer[sensor.first]);
	}

}

void LLInterface::TransmitSensors(int64 microTime, std::map<std::string, double> sensors)
{
	FilterSensors(sensors);

    json content = json::array();
    json sen;
    for (const auto& sensor : filteredSensorBuffer)
    {
        sen = json::object();

        sen["name"] = sensor.first;
        sen["value"] = sensor.second;
        sen["time"] = (double)((microTime / 1000) / 1000.0);

        content.push_back(sen);
    }
    EcuiSocket::SendJson("sensors", content);
}

void LLInterface::UpdateWarningLight(std::map<std::string, double> sensors)
{
    if (sensors.size() < 1)
    {
        sensors = GetAllSensors();
    }


    if (sensors.find("igniter feedback") != sensors.end())
    {
        if (sensors["igniter feedback"] == 0)
        {
            TurnRed();
            warnlightStatus = 2;
        }
        else
        {
            warnlightStatus = 0;
        }
    }
    if (warnlightStatus != 2)
    {
        if (sensors.find("fuelTankPressure") != sensors.end() && sensors.find("oxTankPressure") != sensors.end())
        {
            if (sensors["fuelTankPressure"] >= 5.0 || sensors["oxTankPressure"] >= 5.0)
            {
                TurnYellow();
                warnlightStatus = 1;
            }
            else if (sensors["fuelTankPressure"] < 5.0 && sensors["oxTankPressure"] < 5.0)
            {
                TurnGreen();
                warnlightStatus = 0;
            }
        }
    }

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
    warnLight->SetMode("spin");
    warnLight->StopBuzzer();
}

void LLInterface::TurnYellow()
{
    warnLight->SetColor(255, 255, 0);
    warnLight->SetMode("spin");
    warnLight->StopBuzzer();
}

void LLInterface::BeepRed()
{
    warnLight->SetColor(255, 0, 0);
    warnLight->SetMode("blink");
    warnLight->StartBuzzerBeep(500);
}
