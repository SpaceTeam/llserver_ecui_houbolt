//
// Created by Markus on 03.04.21.
//

#include "Config.h"
#include "can/CANManager.h"

CANManager* CANManager::instance = nullptr;

CANManager::~CANManager()
{
    delete instance;
    initialized = false;
}

CANManager *CANManager::Instance()
{
    if (instance == nullptr)
    {
        instance = new CANManager();
    }
    return instance;
}


CANResult CANManager::Init()
{
    if (!initialized)
    {
        Debug::print("Initializing CANDriver...");
        //arbitration bus parameters
        int32_t tq = std::get<int>(Config::getData("CAN/BUS/ARBITRATION/time_quanta"));
        int32_t phase1 = std::get<int>(Config::getData("CAN/BUS/ARBITRATION/phase1"));
        int32_t phase2 = std::get<int>(Config::getData("CAN/BUS/ARBITRATION/phase2"));
        int32_t sjw = std::get<int>(Config::getData("CAN/BUS/ARBITRATION/sync_jump_width"));
        int32_t prop = std::get<int>(Config::getData("CAN/BUS/ARBITRATION/propagation_segment"));
        int32_t presc = std::get<int>(Config::getData("CAN/BUS/ARBITRATION/prescaler"));
        //data bus parameters
        int32_t tqData = std::get<int>(Config::getData("CAN/BUS/DATA/time_quanta"));
        int32_t phase1Data = std::get<int>(Config::getData("CAN/BUS/DATA/phase1"));
        int32_t phase2Data = std::get<int>(Config::getData("CAN/BUS/DATA/phase2"));
        int32_t sjwData = std::get<int>(Config::getData("CAN/BUS/DATA/sync_jump_width"));
        int32_t propData = std::get<int>(Config::getData("CAN/BUS/DATA/propagation_segment"));
        int32_t prescData = std::get<int>(Config::getData("CAN/BUS/DATA/prescaler"));
        canDriver = new CANDriver(std::bind(&CANManager::OnCANRecv, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3),
                std::bind(&CANManager::OnCANError, this));
        Debug::print("Initializing CANDriver...");

        initialized = true;
    }
    else
    {
        Debug::error("CANManager already initialized");
        return CANResult::ERROR;
    }
    return CANResult::SUCCESS;
}

std::vector<std::string> CANManager::GetChannelStates()
{
    return nullptr;
}

std::map<std::string, std::function<CANResult(...)>> CANManager::GetChannelCommands()
{
    return nullptr;
}

std::map<std::string, double> CANManager::GetLatestSensorData()
{
    return nullptr;
}

void CANManager::OnChannelStateChanged(std::string, double)
{

}

void CANManager::OnCANRecv(uint32_t canID, uint8_t *payload, uint32_t payloadLength)
{

}

void CANManager::OnCANError()
{

}
