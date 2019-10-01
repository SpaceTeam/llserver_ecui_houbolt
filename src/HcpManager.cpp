//
// Created by Markus on 2019-09-30.
//

#include "utils.h"
#include "config.h"
#include "HcpCommands.h"

#include "HcpManager.h"

using json = nlohmann::json;

using namespace std;

Serial* HcpManager::hcpSerial;
json HcpManager::mapping = nullptr;
uint8 HcpManager::lastServoPosArr[SERVO_COUNT] = {0};
bool HcpManager::servoEnabledArr[SERVO_COUNT] = {false};

void HcpManager::init()
{
    //hcpSerial = new Serial(HCP_DEVICE, HCP_BAUD_RATE);
    LoadMapping();
}

//TODO: check if connected
void HcpManager::restart()
{
    delete hcpSerial;
    hcpSerial = new Serial(HCP_DEVICE, HCP_BAUD_RATE);
}

void HcpManager::LoadMapping()
{
    mapping = json::parse(utils::loadFile(MAPPING_FILE_PATH));
    Debug::info("mapping loaded");
}

void HcpManager::SaveMapping()
{
    utils::saveFile(MAPPING_FILE_PATH, mapping.dump(4));
    Debug::info("mapping saved");
}

bool HcpManager::CheckPort(uint8 port, Device_Type type)
{

    if (type == Device_Type::SERVO)
    {
        if (port < SERVO_COUNT)
        {
            return true;
        }
        else
        {
            Debug::error("Servo Port %d above maxium port number", port);
            return false;
        }
    }
    else if (type == Device_Type::ANALOG)
    {
        if (port < ANALOG_COUNT)
        {
            return true;
        }
        else
        {
            Debug::error("Analog Port %d above maxium port number", port);
            return false;
        }
    }
    else if (type == Device_Type::DIGITAL)
    {
        if (port < DIGITAL_COUNT)
        {
            return true;
        }
        else
        {
            Debug::error("Digital Port %d above maxium port number", port);
            return false;
        }
    }

}

json HcpManager::FindObjectByName(std::string name, Device_Type type)
{
    json device = nullptr;

    if (mapping != nullptr)
    {
        string typeName;
        if (type == Device_Type::SERVO)
        {
            typeName = "servo";
        }
        else if (type == Device_Type::ANALOG)
        {
            typeName = "analog";
        }
        else if (type == Device_Type::DIGITAL)
        {
            typeName = "digital";
        }

        for (auto it = mapping[typeName].begin(); it != mapping[typeName].end(); ++it)
        {
            if (it.key().compare(name) == 0)
            {
                device = it.value();
                break;
            }
        }
    }
    else
    {
        Debug::error("Mapping is null");
    }
    return device;
}

json HcpManager::FindObjectByPort(uint8 port, Device_Type type)
{
    json device = nullptr;

    if (mapping != nullptr)
    {
        string typeName;
        if (type == Device_Type::SERVO)
        {
            typeName = "servo";
        }
        else if (type == Device_Type::ANALOG)
        {
            typeName = "analog";
        }
        else if (type == Device_Type::DIGITAL)
        {
            typeName = "digital";
        }

        for (auto dev : mapping[typeName])
        {
            if (dev["port"] == port)
            {
                device = dev;
                break;
            }
        }
    }
    else
    {
        Debug::error("Mapping is null");
    }
    return device;
}

bool HcpManager::ExecCommand(std::string name, uint8 percent)
{
    bool success = false;

    string typeName;
    json device = nullptr;

    if (mapping != nullptr)
    {
        for (auto typeIt = mapping.begin(); typeIt != mapping.end(); ++typeIt)
        {
            for (auto it = typeIt.value().begin(); it != typeIt.value().end(); ++it)
            {
                if (it.key().compare(name) == 0)
                {
                    typeName = typeIt.key();
                    device = it.value();
                    break;
                }
            }
            if (device != nullptr)
            {
                break;
            }
        }

        if (device != nullptr)
        {
            if (typeName.compare("servo") == 0)
            {
                SetServo(device, percent);
            }
//            else if (typeName.compare("motor") == 0)
//            {
//                SetMotor(device, percent);
//            }
            else
            {
                Debug::error("unknown type name");
            }
        }
    }
    else
    {
        Debug::error("Mapping is null");
    }

    return success;
}

bool HcpManager::EnableServo(uint8 port)
{
    if (CheckPort(port, Device_Type::SERVO))
    {
        servoEnabledArr[port] = true;
        return SetServoRaw(port, lastServoPosArr[port]);
    }
    return false;
}

bool HcpManager::DisableServo(uint8 port)
{
    if (CheckPort(port, Device_Type::SERVO))
    {
        servoEnabledArr[port] = true;
        return SetServoRaw(port, lastServoPosArr[port]);
    }
    return false;
}

bool HcpManager::SetServoRaw(uint8 port, uint16 onTime)
{
    bool success = false;

    if (CheckPort(port, Device_Type::SERVO))
    {
        //TODO: check if this is okay or if it belongs inside HCP_OK block
        lastServoPosArr[port] = onTime;

        HCP_MSG msg;
        msg.optcode = HCP_SERVO;
        msg.payloadSize = 3;
        msg.payload = new uint8[msg.payloadSize];

        uint8 highOnTime = onTime >> 8;
        if (servoEnabledArr[port])
        {
            highOnTime |= 0x80;
        }

        uint8 lowOnTime = onTime & 0x00FF;

        msg.payload[0] = port;
        msg.payload[1] = highOnTime;
        msg.payload[2] = lowOnTime;


        Debug::info("set servo %d to %d", port, onTime);

        hcpSerial->Write(msg);
        HCP_MSG rep = hcpSerial->ReadSync();

        if (rep.optcode == HCP_OK)
        {
            success = true;
        }

        delete[] msg.payload;
    }
    return success;
}

bool HcpManager::SetServo(string name, uint8 percent)
{
    bool success = false;

    json device = FindObjectByName(name, Device_Type::SERVO);

    if (device != nullptr)
    {
        success = SetServo(device, percent);
    }
    else
    {
        Debug::error("name not found");
    }
    return success;
}

bool HcpManager::SetServo(uint8 port, uint8 percent)
{
    bool success = false;

    json device = FindObjectByPort(port, Device_Type::SERVO);

    if (device != nullptr)
    {
        success = SetServo(device, percent);
    }
    else
    {
        Debug::error("port not found");
    }
    return success;
}

bool HcpManager::SetServo(json device, uint8 percent)
{
    bool success = false;

    if (device != nullptr)
    {
        uint8 port = device["port"];
        if (CheckPort(port, Device_Type::SERVO))
        {
            percent = clamp((int)percent, 0, 100);

            uint16 endpoints[2];

//            if (utils::keyExists(device, "feedbackEndpoints"))
//            {
//                endpoints[0] = device["feedbackEndpoints"][0];
//                endpoints[1] = device["feedbackEndpoints"][1];
//            }
//            else
//            {
//                endpoints[0] = device["endpoints"][0];
//                endpoints[1] = device["endpoints"][1];
//            }

            endpoints[0] = device["endpoints"][0];
            endpoints[1] = device["endpoints"][1];

            uint16 onTime = ((endpoints[1] - endpoints[0]) / 100.0) * percent;
            success = SetServoRaw(port, onTime);
        }
        else
        {
            Debug::error("Port not valid in mapping");
        }
    }
    else
    {
        Debug::error("device not found");
    }
    return success;
}

uint16 HcpManager::GetAnalog(std::string name)
{
    uint16 value = -1;

    json device = FindObjectByName(name, Device_Type::ANALOG);

    if (device != nullptr)
    {
        value = GetAnalog((uint8)device["port"]);
    }
    else
    {
        Debug::error("name not found");
    }

    return value;
}

uint16 HcpManager::GetAnalog(uint8 port)
{
    uint16 value = -1;

    if (CheckPort(port, Device_Type::ANALOG))
    {
        HCP_MSG msg;
        msg.optcode = HCP_ANALOG_REQ;
        msg.payloadSize = 1;
        msg.payload = new uint8[msg.payloadSize];;

        msg.payload[0] = port;

        Debug::info("get analog %d", port);

        hcpSerial->Write(msg);
        HCP_MSG rep = hcpSerial->ReadSync();

        if (rep.optcode == HCP_ANALOG_REP)
        {
            if (rep.payload[0] == port)
            {
                value = (rep.payload[1] << 8) + rep.payload[2];
            }
            else
            {
                Debug::error("Ports of Analog REQ and REP are not the same");
            }
        }
        delete[] msg.payload;
    }
    else
    {
        Debug::error("Port not valid in mapping");
    }
    return value;
}

uint8 HcpManager::GetDigital(std::string name)
{
    uint8 state = -1;

    json device = FindObjectByName(name, Device_Type::DIGITAL);

    if (device != nullptr)
    {
        state = GetDigital((uint8)device["port"]);
    }
    else
    {
        Debug::error("name not found");
    }

    return state;
}

uint8 HcpManager::GetDigital(uint8 port)
{
    uint8 state = -1;

    if (CheckPort(port, Device_Type::DIGITAL))
    {
        HCP_MSG msg;
        msg.optcode = HCP_DIGITAL_REQ;
        msg.payloadSize = 1;
        msg.payload[msg.payloadSize];

        msg.payload[0] = port;

        Debug::info("get digital %d", port);

//        hcpSerial->Write(msg);
//        HCP_MSG rep = hcpSerial->ReadSync();
//
//        if (rep.optcode == HCP_DIGITAL_REP)
//        {
//            if (rep.payload[0] == port)
//            {
//                state = rep.payload[1];
//            }
//            else
//            {
//                Debug::error("Ports of Digital REQ and REP are not the same");
//            }
//        }
    }
    else
    {
        Debug::error("Port not valid in mapping");
    }
    return state;
}

uint16 HcpManager::GetBatteryLevel()
{
    return 0;
}
