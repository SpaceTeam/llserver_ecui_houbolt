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
Mapping* HcpManager::mapping;
//TODO: set default position at the beginning instead
uint16 HcpManager::lastServoPosArr[SERVO_COUNT] = {1000};
bool HcpManager::servoEnabledArr[SERVO_COUNT] = {false};

std::recursive_mutex HcpManager::serialMtx;

void HcpManager::init()
{
    hcpSerial = new Serial(HCP_DEVICE, HCP_BAUD_RATE);
    mapping = new Mapping(HCP_MAPPING_FILE_PATH);
}

//TODO: check if connected
void HcpManager::restart()
{
    delete hcpSerial;
    hcpSerial = new Serial(HCP_DEVICE, HCP_BAUD_RATE);
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
    else if (type == Device_Type::MOTOR)
    {
        if (port < MOTOR_COUNT)
        {
            return true;
        }
        else
        {
            Debug::error("Motor Port %d above maxium port number", port);
            return false;
        }
    }
    //NOTE : currently digital out is using motor ports!
    else if (type == Device_Type::DIGITAL_OUT)
    {
        if (port < MOTOR_COUNT)
        {
            return true;
        }
        else
        {
            Debug::error("Digital Output Port %d above maxium port number", port);
            return false;
        }
    }
    else if (type == Device_Type::ANALOG)
    {
        if (port < ANALOG_COUNT || port == HCP_ANALOG_SUPPLY_PORT)
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
    return false;
}

std::vector<std::string> HcpManager::GetAllSensorNames()
{
    vector<std::string> sensorNames;

    json analogs = mapping->GetDevices(Device_Type::ANALOG);
    json digitals = mapping->GetDevices(Device_Type::DIGITAL);
    if (analogs != nullptr)
    {
        for (auto it = analogs.begin(); it != analogs.end(); ++it)
        {
            if (utils::keyExists(it.value(), "loadCells"))
            {
                sensorNames.push_back(it.key() + " 1");
                sensorNames.push_back(it.key() + " 2");
                sensorNames.push_back(it.key() + " 3");
            }
            else
            {
                sensorNames.push_back(it.key());
            }

        }
    }
    else
    {
        Debug::error("No analogs found");
    }

    if (digitals != nullptr)
    {
        for (auto it = digitals.begin(); it != digitals.end(); ++it)
        {
            sensorNames.push_back(it.key());
        }
    }
    else
    {
        Debug::error("No digitals found");
    }

    std::sort(sensorNames.begin(), sensorNames.end());
    return sensorNames;
}

std::map<std::string, int32> HcpManager::GetAllSensors()
{
    map<std::string, int32> sensors;

    json analogs = mapping->GetDevices(Device_Type::ANALOG);
    json digitals = mapping->GetDevices(Device_Type::DIGITAL);
    if (analogs != nullptr)
    {
        for (auto it = analogs.begin(); it != analogs.end(); ++it)
        {
            if (utils::keyExists(it.value(), "loadCells"))
            {
                int32* cells = GetLoadCells();

                sensors[it.key() + " 1"] = cells[0];
                sensors[it.key() + " 2"] = cells[1];
                sensors[it.key() + " 3"] = cells[2];
            }
            else
            {
                sensors[it.key()] = GetAnalog(it.key());
            }

        }
    }
    else
    {
        Debug::error("No analogs found");
    }

    if (digitals != nullptr)
    {
        for (auto it = digitals.begin(); it != digitals.end(); ++it)
        {
            sensors[it.key()] = GetDigital(it.key());
        }
    }
    else
    {
        Debug::error("No digitals found");
    }

    return sensors;
}

json HcpManager::GetAllServoData()
{
    json data = json::array();

    json servos = mapping->GetDevices(Device_Type::SERVO);
    if (servos != nullptr)
    {
        json currServo;
        for (auto it = servos.begin(); it != servos.end(); ++it)
        {
            currServo = json::object();
            currServo["name"] = it.key();
            currServo["endpoints"] = it.value()["endpoints"];

            data.push_back(currServo);
        }
    }
    else
    {
        Debug::error("No Servos found");
    }

    return data;
}

bool HcpManager::ExecCommand(std::string name, uint8 percent)
{
    bool success = false;

    Device_Type type = mapping->GetTypeByName(name);
    json device = mapping->GetDeviceByName(name, type);

    if (device != nullptr)
    {
        if (type == Device_Type::SERVO)
        {
            success = SetServo(device, percent);
        }
        else if (type == Device_Type::MOTOR)
        {
            uint8 port = device["port"];
            success = SetMotor(port, (int8)percent);
        }
        else if (type == Device_Type::DIGITAL_OUT)
        {

            uint8 port = device["port"];
            success = SetDigitalOutputs(port, (bool)percent);
        }
        else
        {
            Debug::error("unknown type name");
        }
    }
    else
    {
        Debug::error("no device with name '" + name + "' found");
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
        servoEnabledArr[port] = false;
        return SetServoRaw(port, lastServoPosArr[port]);
    }
    return false;
}

bool HcpManager::EnableAllServos()
{
    bool success = true;

    bool currSuccess;
    for (uint8 i = 0; i < SERVO_COUNT; i++)
    {
        currSuccess = EnableServo(i);
        if (!currSuccess)
        {
            success = false;
        }
    }

    return success;
}

bool HcpManager::DisableAllServos()
{
    bool success = true;

    bool currSuccess;
    for (uint8 i = 0; i < SERVO_COUNT; i++)
    {
        currSuccess = DisableServo(i);
        if (!currSuccess)
        {
            success = false;
        }
    }
    return success;
}

void HcpManager::SetServoMin(std::string name, uint16 min)
{
    json servos = mapping->GetDevices(Device_Type::SERVO);
    json analogs = mapping->GetDevices(Device_Type::ANALOG);

    if (utils::keyExists(servos, name))
    {
        json servo = servos[name];
        if (utils::keyExists(servo, "feedbackAnalog"))
        {
            string fbckName = servo["feedbackAnalog"];
            if (utils::keyExists(analogs, fbckName))
            {
                json fbckAnalog = analogs[fbckName];

                bool success = SetServoRaw((uint8)servo["port"], min);
                if (success)
                {
                    uint16 fbckValue = GetAnalog((uint8)fbckAnalog["port"]);

                    //set new feedback endpoint
                    servo["feedbackEndpoints"][0] = fbckValue;
                }
            }
            else
            {
                Debug::error("no analog sensor with name " + fbckName + " as feedback found");
            }

        }
        else
        {
            Debug::info("no feedback label found");
        }
        servo["endpoints"][0] = min;
	    mapping->SetDevice(name, servo, Device_Type::SERVO);
    }
    else
    {
        Debug::error("no servo with name '" + name + "' found");
    }

}

void HcpManager::SetServoMax(std::string name, uint16 max)
{

    json servos = mapping->GetDevices(Device_Type::SERVO);
    json analogs = mapping->GetDevices(Device_Type::ANALOG);

    if (utils::keyExists(servos, name))
    {
        json servo = servos[name];
        if (utils::keyExists(servo, "feedbackAnalog"))
        {
            string fbckName = servo["feedbackAnalog"];
            if (utils::keyExists(analogs, fbckName))
            {
                json fbckAnalog = analogs[fbckName];

                bool success = SetServoRaw((uint8)servo["port"], max);
                if (success)
                {
                    uint16 fbckValue = GetAnalog((uint8)fbckAnalog["port"]);

                    //set new feedback endpoint
                    servo["feedbackEndpoints"][1] = fbckValue;
                }
            }
            else
            {
                Debug::error("no analog sensor with name " + fbckName + " as feedback found");
            }

        }
        else
        {
            Debug::info("no feedback label found");
        }
        servo["endpoints"][1] = max;
	    mapping->SetDevice(name, servo, Device_Type::SERVO);
    }
    else
    {
        Debug::error("no servo with name '" + name + "' found");
    }
}

bool HcpManager::SetServoRaw(std::string name, uint16 onTime)
{
    bool success = false;

    json device = mapping->GetDeviceByName(name, Device_Type::SERVO);

    if (device != nullptr)
    {
        uint8 port = device["port"];
        success = SetServoRaw(port, onTime);
    }
    else
    {
        Debug::error("name not found");
    }
    return success;
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

	    uint16 dblOnTime = onTime * 2;
	    if (dblOnTime <= SERVO_MAX_ONTIME)
        {
	        uint8 highOnTime = dblOnTime >> 8;
            if (servoEnabledArr[port])
            {
                highOnTime |= 0x80;
            }

            uint8 lowOnTime = dblOnTime & 0x00FF;

            msg.payload[0] = port;
            msg.payload[1] = highOnTime;
            msg.payload[2] = lowOnTime;


            Debug::info("set servo %d to %d", port, onTime);

            serialMtx.lock();
            hcpSerial->Write(msg);
            HCP_MSG* rep = hcpSerial->ReadSync();
            serialMtx.unlock();

            if (rep != nullptr)
            {
                if (rep->optcode == HCP_OK)
                {
                    success = true;
                }
		else
		{
		    Debug::error("REP yields optcode %x", rep->optcode);
		}
                delete rep->payload;
                delete rep;
            }
            else
            {
                Debug::error("hcp response message is null");
            }
        }
        else
        {
            Debug::error("onTime longer than maximum allowed");
        }

        delete[] msg.payload;
    }
    else
    {
        Debug::error("Servo Port %d not valid in mapping", port);
    }
    return success;
}

bool HcpManager::SetServo(string name, uint8 percent)
{
    bool success = false;

    json device = mapping->GetDeviceByName(name, Device_Type::SERVO);

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

    json device = mapping->GetDeviceByPort(port, Device_Type::SERVO);

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

            int32 endpoints[2];

            endpoints[0] = device["endpoints"][0];
            endpoints[1] = device["endpoints"][1];

            uint16 onTime = (((endpoints[1] - endpoints[0]) / 100.0) * percent) + endpoints[0];
            success = SetServoRaw(port, onTime);
        }
        else
        {
            Debug::error("Servo Port %d not valid in mapping", port);
        }
    }
    else
    {
        Debug::error("device not found");
    }
    return success;
}

bool HcpManager::SetMotor(uint8 port, int8 percent)
{
    return SetMotorRaw(port, Motor_Mode::POWER, percent*10);
}

bool HcpManager::SetMotor(std::string name, Motor_Mode mode, int16 amount)
{
    bool success = false;

    json device = mapping->GetDeviceByName(name, Device_Type::MOTOR);

    if (device != nullptr)
    {
        uint8 port = device["port"];
        success = SetMotorRaw(port, mode, amount);
    }
    else
    {
        Debug::error("port not found");
    }
    return success;
}

bool HcpManager::SetMotorRaw(uint8 port, Motor_Mode mode, int16 amount)
{
    bool success = false;

    if (CheckPort(port, Device_Type::MOTOR))
    {
        amount = clamp((int)amount, -1000, 1000);

        HCP_MSG msg;
        msg.optcode = HCP_MOTOR;
        msg.payloadSize = 4;
        msg.payload = new uint8[msg.payloadSize];

        uint8 highAmount = amount >> 8;

        uint8 lowAmount = amount & 0x00FF;

        msg.payload[0] = port;
        msg.payload[1] = (uint8) mode;
        msg.payload[2] = highAmount;
        msg.payload[3] = lowAmount;

        Debug::print("set motor %d to %d", port, amount);

        serialMtx.lock();
        hcpSerial->Write(msg);
        HCP_MSG* rep = hcpSerial->ReadSync();
        serialMtx.unlock();

        if (rep != nullptr)
        {
            if (rep->optcode == HCP_OK)
            {
                success = true;
            }
	    else
	    {
	        printf("REP yields optcode %x", rep->optcode);
	    }
            delete rep->payload;
            delete rep;
        }
        else
        {
            Debug::error("hcp response message is null");
        }
        delete[] msg.payload;
    }
    else
    {
        Debug::error("Motor Port %d not valid in mapping", port);
    }

    return success;
}

bool HcpManager::SetDigitalOutputs(std::string name, bool enable)
{
    bool success = false;

    json device = mapping->GetDeviceByName(name, Device_Type::DIGITAL_OUT);

    if (device != nullptr)
    {
        uint8 port = device["port"];
        SetDigitalOutputs(port, enable);
    }
    else
    {
        Debug::error("digital output not found");
    }
    return success;
}

//careful: digitalOut and motor share the same ports
bool HcpManager::SetDigitalOutputs(uint8 port, bool enable)
{
    bool success = false;

    if (CheckPort(port, Device_Type::DIGITAL_OUT))
    {
        Debug::info("set digital output %d to %d", port, enable);
        if (enable)
        {
            success = SetMotorRaw(port, Motor_Mode::POWER, 1000);
        }
        else
        {
            success = SetMotorRaw(port, Motor_Mode::POWER, 0);
        }
    }
    else
    {
        Debug::error("Digital Output Port %d not valid in mapping", port);
    }

    return success;
}

int32 *HcpManager::GetLoadCells()
{

    int32 *value = new int32[3];
    std::fill( value, value+3, -1 );


    HCP_MSG msg;
    msg.optcode = HCP_ST_THRUST_REQ;
    msg.payloadSize = 0;
    msg.payload = nullptr;

    Debug::info("get load cells");

    serialMtx.lock();
    hcpSerial->Write(msg);
    HCP_MSG* rep = hcpSerial->ReadSync();
    serialMtx.unlock();

    if (rep != nullptr)
    {
        if (rep->optcode == HCP_ST_THRUST_REP)
        {
            uint8 signByte1 = 0, signByte2 = 0, signByte3 = 0;
            if (rep->payload[0] >= 0x80)
            {
                signByte1 = 0xFF;
            }
            if (rep->payload[3] >= 0x80)
            {
                signByte2 = 0xFF;
            }
            if (rep->payload[6] >= 0x80)
            {
                signByte3 = 0xFF;
            }
            value[0] = signByte1 << 24 | (rep->payload[0] << 16) | (rep->payload[1] << 8) | rep->payload[2];
            value[1] = signByte2 << 24 | (rep->payload[3] << 16) | (rep->payload[4] << 8) | rep->payload[5];
            value[2] = signByte3 << 24 | (rep->payload[6] << 16) | (rep->payload[7] << 8) | rep->payload[8];

            Debug::info("REP Cell 1: %d", value[0]);
            Debug::info("REP Cell 2: %d", value[1]);
            Debug::info("REP Cell 3: %d", value[2]);
        }
        else
        {
            Debug::info("Other REP opcode than expected: %x", rep->optcode);
        }
        delete rep->payload;
        delete rep;
    }
    else
    {
        Debug::error("hcp response message is null");
    }
    delete[] msg.payload;

    return value;
}

int32 HcpManager::GetAnalog(std::string name)
{
    int32 value = -1;

    json device = mapping->GetDeviceByName(name, Device_Type::ANALOG);

    if (device != nullptr)
    {
        value = GetAnalog((uint8)device["port"]);

        if (utils::keyExists(device, "servo"))
        {
            Debug::info("converting feedback sensor to percentage");
            //get servo of fbck sensor

            string servoName = device["servo"];
            json servo = mapping->GetDeviceByName(servoName, Device_Type::SERVO);

            vector<uint16> servoEndpoints = servo["endpoints"];
            vector<uint16> fbckEndpoints = servo["feedbackEndpoints"];

            double norm = (((value-fbckEndpoints[0])*1.0) / (fbckEndpoints[1] - fbckEndpoints[0]));

            //convert to us
            //value = ((servoEndpoints[1] - servoEndpoints[0])*norm) + servoEndpoints[0];

            //convert to percentage
            value = norm * 100;
        }
        else if (utils::keyExists(device, "map"))
        {
            Debug::info("mapping sensor value");

            json map = device["map"];

            int32 before = value;
            value = ((double)value - (double)map["d"]) * (double)map["k"];

            Debug::info("from %d to %d", before, value);
        }
    }
    else
    {
        Debug::error("name not found");
    }

    return value;
}

int32 HcpManager::GetAnalog(uint8 port)
{
    int32 value = -1;

    if (CheckPort(port, Device_Type::ANALOG))
    {
        HCP_MSG msg;
        msg.optcode = HCP_ANALOG_REQ;
        msg.payloadSize = 1;
        msg.payload = new uint8[msg.payloadSize];

        msg.payload[0] = port;

        Debug::info("get analog %d", port);

        serialMtx.lock();
        hcpSerial->Write(msg);
        HCP_MSG* rep = hcpSerial->ReadSync();
        serialMtx.unlock();

        if (rep != nullptr)
        {
            if (rep->optcode == HCP_ANALOG_REP)
            {
                Debug::info("REP Port %d", rep->payload[0]);
                Debug::info("REP Val %d", rep->payload[1]);

                if (rep->payload[0] == port)
                {

                    value = (rep->payload[1] << 8) | rep->payload[2];
                }
                else
                {
                    Debug::error("Ports of Analog REQ and REP are not the same");
                }
            }
	    else
	    {
		Debug::info("Other REP opcode than expected: %x", rep->optcode); 
	    }
            delete rep->payload;
            delete rep;
        }
        else
        {
            Debug::error("hcp response message is null");
        }
        delete[] msg.payload;
    }
    else
    {
        Debug::error("Analog Port %d not valid in mapping", port);
    }
    return value;
}

uint8 HcpManager::GetDigital(std::string name)
{
    uint8 state = -1;

    json device = mapping->GetDeviceByName(name, Device_Type::DIGITAL);

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
        msg.payload = new uint8[msg.payloadSize];

        msg.payload[0] = port;

        Debug::info("get digital %d", port);

        serialMtx.lock();
        hcpSerial->Write(msg);
        HCP_MSG* rep = hcpSerial->ReadSync();
        serialMtx.unlock();

        if (rep != nullptr)
        {
            if (rep->optcode == HCP_DIGITAL_REP)
            {
                Debug::info("REP Port %d", rep->payload[0]);
                Debug::info("REP Val %d", rep->payload[1]);


                if (rep->payload[0] == port)
                {
                    state = rep->payload[1];
                }
                else
                {
                    Debug::error("Ports of Digital REQ and REP are not the same");
                }
            }
	    else
	    {
		Debug::info("Other REP opcode than expected: %x", rep->optcode); 
	    }

            delete rep->payload;
            delete rep;
        }
        else
        {
            Debug::error("hcp response message is null");
        }
        delete[] msg.payload;
    }
    else
    {
        Debug::error("Digital Port %d not valid in mapping", port);
    }
    return state;
}

uint16 HcpManager::GetBatteryLevel()
{
    return GetAnalog(HCP_ANALOG_SUPPLY_PORT);
}
