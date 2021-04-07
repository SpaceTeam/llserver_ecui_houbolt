//
// Created by Markus on 2019-10-15.
//

//#include "hcp/HcpManager.h"
#include "can/CANManager.h"
#include "EcuiSocket.h"
#include "Timer.h"
#include "Config.h"

#include "LLInterface.h"

I2C* LLInterface::i2cDevice;
WarnLight* LLInterface::warnLight;
TMPoE *LLInterface::tmPoE;

CANManager *LLInterface::canManager;
EventManager *LLInterface::eventManager;
StateController *LLInterface::stateController;

DataFilter *LLInterface::dataFilter;

//GPIO[] LLInterface::gpioDevices;

//SPI* LLInterface::spiDevice;

bool LLInterface::isInitialized = false;

bool LLInterface::useTMPoE = false;

bool LLInterface::isTransmittingStates = false;
int32_t LLInterface::warnlightStatus = -1;
Timer* LLInterface::stateTimer;
Timer* LLInterface::sensorTimer;


void LLInterface::Init()
{
    if (!isInitialized)
    {
//        Debug::print("Initializing HcpManager...");
//        HcpManager::Init();
//        Debug::print("Initializing HcpManager done\n");
//        Debug::print("Starting periodic sensor fetching...");
//        HcpManager::StartSensorFetch(std::get<int>(Config::getData("HCP/sensor_sample_rate")));
//        Debug::print("Periodic sensor fetching started\n");

        Debug::print("Initializing EventManager...");
        eventManager = EventManager::Instance();
        eventManager->Init();
        Debug::print("Initializing EventManager done\n");

        Debug::print("Initializing StateController...");
        stateController = StateController::Instance();
        stateController->Init(std::bind(&EventManager::OnStateChange, eventManager, std::placeholders::_1, std::placeholders::_2));
        Debug::print("Initializing StateController done\n");

        Debug::print("Initializing CANManager...");
        canManager = CANManager::Instance();
        canManager->Init();
        Debug::print("Initializing CANManager done\n");

        Debug::print("Initializing DataFilter...");
        double sensorsSmoothingFactor = std::get<double>(Config::getData("WEBSERVER/sensors_smoothing_factor"));
        dataFilter = new DataFilter(sensorsSmoothingFactor);
        Debug::print("Initializing DataFilter done\n");

        stateTimer = new Timer(40, "stateTimer");
        sensorTimer = new Timer(40, "sensorTimer");
        uint64_t sensorSamplingRate = std::get<int>(Config::getData("LLSERVER/sensor_sampling_rate"));
        sensorTimer->startContinous(0, sensorSamplingRate, LLInterface::FilterSensors, LLInterface::StopFilterSensors);
        Debug::print("Connecting to warning light...");
        warnLight = new WarnLight(0);

        useTMPoE = std::get<bool>(Config::getData("useTMPoE"));
        if (useTMPoE)
        {
            Debug::print("Initializing TMPoE...");
            tmPoE = new TMPoE(0, 50);
        }
        //i2cDevice = new I2C(0, "someDev"); //not in use right now



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
        //TODO: MP destruct CANManager?
    }
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
        std::vector<uint32_t> tmpValues = tmPoE->Read();
        for (uint32_t i = 0; i < tmpValues.size(); i++)
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

bool LLInterface::ExecCommand(std::string name, nlohmann::json value)
{
    if (value.type() != nlohmann::json::value_t::number_float &&
        value.type() != nlohmann::json::value_t::number_integer &&
        value.type() != nlohmann::json::value_t::number_unsigned)
    {
        return false;
    }
    return HcpManager::ExecCommand(name, (uint8_t)value);
}

void LLInterface::StartStateTransmission()
{
    if (!isTransmittingStates)
    {
        uint64_t stateSamplingRate = std::get<int>(Config::getData("LLSERVER/state_sampling_rate"));
        isTransmittingStates = true;
        stateTimer->startContinous(0, stateSamplingRate, LLInterface::GetStates, LLInterface::StopGetStates);
    }
}

void LLInterface::StopStateTransmission()
{
    if (isTransmittingStates)
    {

        stateTimer->stop();
        isTransmittingStates = false;
    }
}

void LLInterface::StopGetStates()
{

}

void LLInterface::GetStates(int64_t microTime)
{
    std::map<std::string, double> states = stateController->GetDirtyStates();

    TransmitStates(microTime, states);

}

void LLInterface::FilterSensors(int64_t microTime)
{
    std::map<std::string, double> rawSensors;
    rawSensors = canManager->GetLatestSensorData();

    std::map<std::string, double> filteredSensors;
    filteredSensors = dataFilter->FilterData(rawSensors);



}

void LLInterface::TransmitStates(int64_t microTime, std::map<std::string, double> &states)
{

    nlohmann::json content = nlohmann::json::array();
    nlohmann::json statesJson;
    for (const auto& state : states)
    {
        stateJson = nlohmann::json::object();

        stateJson["name"] = state.first;
        stateJson["value"] = state.second;
        stateJson["time"] = (double)((microTime / 1000) / 1000.0);

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
