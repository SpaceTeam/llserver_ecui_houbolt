//
// Created by Markus on 2019-09-30.
//

#include "utils.h"
#include "config_old.h"
#include "Config.h"
#include "hcp/HcpCommands.h"


#include "hcp/HcpManager.h"

#define HCP_THRUST_SENSORS_COUNT 3


using json = nlohmann::json;

using namespace std;

Serial* HcpManager::hcpSerial;
HcpMapping* HcpManager::mapping;
//TODO: set default position at the beginning instead
uint16_t HcpManager::lastServoPosArr[SERVO_COUNT] = {0};
bool HcpManager::servoEnabledArr[SERVO_COUNT] = {false};

std::mutex HcpManager::sensorMtx;
Timer *HcpManager::sensorTimer;
std::map<std::string, double> HcpManager::sensorBuffer;

std::recursive_mutex HcpManager::serialMtx;

void HcpManager::Init()
{
    string hcpDevice = std::get<std::string>(Config::getData("HCP/device"));
    int32_t baudrate = std::get<int>(Config::getData("HCP/baudrate"));

	string mappingPath = std::get<std::string>(Config::getData("mapping_path"));
    
	hcpSerial = new Serial(hcpDevice, baudrate);
    mapping = new HcpMapping(mappingPath);

    sensorTimer = new Timer(41, "HcpTimer");
    //TODO: this is only valid for the hedgehog llserver, change for other platforms (analog count doesn't have to
    //be the same as digital count) and + 1 for battery value

    for (uint8_t i = 0; i < SERVO_COUNT; i++)
    {
        nlohmann::json device = mapping->GetDeviceByPort(i, Device_Type::SERVO);
        if (device != nullptr)
        {
            int16_t closedPos;

            closedPos = device["endpoints"][0];

            SetServoRaw(i, closedPos);
        }
    }
}

//TODO: check if connected
void HcpManager::Restart()
{
    delete hcpSerial;
    string hcpDevice = std::get<std::string>(Config::getData("HCP/device"));
    int32_t baudrate = std::get<int>(Config::getData("HCP/baudrate"));
    hcpSerial = new Serial(hcpDevice, baudrate);
}

void HcpManager::Destroy()
{
    delete hcpSerial;
    delete mapping;
    delete sensorTimer;
}

bool HcpManager::CheckPort(uint8_t port, Device_Type type)
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

void HcpManager::StartSensorFetch(uint32_t sampleRate)
{
    sensorTimer->startContinous(0, 1000000/sampleRate, FetchSensors);
}

//NOTE: at this point, writing the sensor buffer doesn't occur in one lock segment, meaning that it can be the case
//that some sensor values of the controller get logged before the other ones, although they are read in the same timer tick
void HcpManager::FetchSensors(uint64_t microTime)
{

    nlohmann::json analogs = mapping->GetDevices(Device_Type::ANALOG);
    nlohmann::json digitals = mapping->GetDevices(Device_Type::DIGITAL);
    if (analogs != nullptr)
    {
        for (auto it = analogs.begin(); it != analogs.end(); ++it)
        {
            if (utils::keyExists(it.value(), "loadCells"))
            {
                int32_t* cells = GetLoadCells();

                if (utils::keyExists(it.value(), "map"))
                {
                    Debug::info("mapping thrust values");

                    nlohmann::json maps = it.value()["map"];

                    if (maps.type() == nlohmann::json::value_t::array)
                    {
                        double mappedValues[HCP_THRUST_SENSORS_COUNT] = {-1.0};
                        double mappedSum = 0.0;

                        if (maps.size() > 0)
                        {
                            for (size_t i = 0; i < maps.size(); i++)
                            {
                                double before = mappedValues[i];
                                mappedValues[i] = ((double) cells[i] * (double) maps[i]["k"]) + (double) maps[i]["d"];

                                Debug::info(it.key() + " %d from %d to %d", i+1, before, mappedValues[i]);

                                mappedSum += mappedValues[i];
                            }
                        }
                        else
                        {
                            for (int i = 0; i < HCP_THRUST_SENSORS_COUNT; i++)
                            {
                                mappedValues[i] = cells[i];
                                mappedSum += mappedValues[i];
                            }
                        }

                        sensorMtx.lock();
                        for (int i = 0; i < HCP_THRUST_SENSORS_COUNT; i++)
                        {
                            sensorBuffer[it.key() + " " + to_string(i+1)] = mappedValues[i];
                        }
                        sensorBuffer[it.key() + " Sum"] = mappedSum;
                        sensorMtx.unlock();
                    }
                    else
                    {
                        Debug::error("maps field in mapping is not an array");
                    }

                }
                else
                {
                    double thrustSum = 0.0;
                    sensorMtx.lock();
                    for (int i = 0; i < HCP_THRUST_SENSORS_COUNT; i++)
                    {
                        sensorBuffer[it.key() + " " + to_string(i+1)] = cells[i];
                        thrustSum += cells[i];
                    }
                    sensorBuffer[it.key() + " Sum"] = thrustSum;
                    sensorMtx.unlock();
                }

                delete cells;

            }
            else
            {
//                sensorMtx.lock();
                    sensorBuffer[it.key()] = GetAnalog(it.key());
//                sensorMtx.unlock();
            }

        }


    }
    else
    {
        Debug::error("No analogs found");
    }

    if (digitals != nullptr)
    {
        sensorMtx.lock();
        for (auto it = digitals.begin(); it != digitals.end(); ++it)
        {
            sensorBuffer[it.key()] = GetDigital(it.key());
        }
        sensorMtx.unlock();
    }
    else
    {
        Debug::error("No digitals found");
    }
}

void HcpManager::StopSensorFetch()
{
    sensorTimer->stop();
}

std::vector<std::string> HcpManager::GetAllSensorNames()
{
    vector<std::string> sensorNames;

    nlohmann::json analogs = mapping->GetDevices(Device_Type::ANALOG);
    nlohmann::json digitals = mapping->GetDevices(Device_Type::DIGITAL);
    if (analogs != nullptr)
    {
        for (auto it = analogs.begin(); it != analogs.end(); ++it)
        {
            if (utils::keyExists(it.value(), "loadCells"))
            {
                sensorNames.push_back(it.key() + " 1");
                sensorNames.push_back(it.key() + " 2");
                sensorNames.push_back(it.key() + " 3");
                sensorNames.push_back(it.key() + " Sum");
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

std::map<std::string, double> HcpManager::GetAllSensors()
{
    map<std::string, double> sensors;

    //std::lock_guard<std::mutex> lock(sensorMtx);

    sensors.insert(sensorBuffer.begin(), sensorBuffer.end());

    return sensors;
}

std::vector<std::string> HcpManager::GetAllOutputNames()
{
    vector<std::string> outputNames;

    nlohmann::json servos = mapping->GetDevices(Device_Type::SERVO);
    nlohmann::json motors = mapping->GetDevices(Device_Type::MOTOR);
    nlohmann::json digitals = mapping->GetDevices(Device_Type::DIGITAL_OUT);
    if (servos != nullptr)
    {
        for (auto it = servos.begin(); it != servos.end(); ++it)
        {
            outputNames.push_back(it.key());

        }
    }
    else
    {
        Debug::error("No servos found");
    }
    if (motors != nullptr)
    {
        for (auto it = motors.begin(); it != motors.end(); ++it)
        {
            outputNames.push_back(it.key());

        }
    }
    else
    {
        Debug::error("No motors found");
    }
    if (digitals != nullptr)
    {
        for (auto it = digitals.begin(); it != digitals.end(); ++it)
        {
            outputNames.push_back(it.key());
        }
    }
    else
    {
        Debug::error("No digitals found");
    }

    std::sort(outputNames.begin(), outputNames.end());
    return outputNames;
}

nlohmann::json HcpManager::GetAllServoData()
{
    nlohmann::json data = nlohmann::json::array();

    nlohmann::json servos = mapping->GetDevices(Device_Type::SERVO);
    if (servos != nullptr)
    {
        nlohmann::json currServo;
        for (auto it = servos.begin(); it != servos.end(); ++it)
        {
            currServo = nlohmann::json::object();
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

bool HcpManager::ExecCommand(std::string name, uint8_t percent)
{
    bool success = false;

    Device_Type type = mapping->GetTypeByName(name);
    nlohmann::json device = mapping->GetDeviceByName(name, type);

    if (device != nullptr)
    {
        if (type == Device_Type::SERVO)
        {
            success = SetServo(device, percent);
        }
        else if (type == Device_Type::MOTOR)
        {
            uint8_t port = device["port"];
            success = SetMotor(port, (int8_t)percent);
        }
        else if (type == Device_Type::DIGITAL_OUT)
        {

            uint8_t port = device["port"];
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

bool HcpManager::EnableServo(uint8_t port)
{
    if (CheckPort(port, Device_Type::SERVO))
    {
        servoEnabledArr[port] = true;
        return SetServoRaw(port, lastServoPosArr[port]);
    }
    return false;
}

bool HcpManager::DisableServo(uint8_t port)
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
    for (uint8_t i = 0; i < SERVO_COUNT; i++)
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
    for (uint8_t i = 0; i < SERVO_COUNT; i++)
    {
        currSuccess = DisableServo(i);
        if (!currSuccess)
        {
            success = false;
        }
    }
    return success;
}

void HcpManager::SetServoMin(std::string name, uint16_t min)
{
    nlohmann::json servos = mapping->GetDevices(Device_Type::SERVO);
    nlohmann::json analogs = mapping->GetDevices(Device_Type::ANALOG);

    if (utils::keyExists(servos, name))
    {
        nlohmann::json servo = servos[name];
        if (utils::keyExists(servo, "feedbackAnalog"))
        {
            string fbckName = servo["feedbackAnalog"];
            if (utils::keyExists(analogs, fbckName))
            {
                nlohmann::json fbckAnalog = analogs[fbckName];

                bool success = SetServoRaw((uint8_t)servo["port"], min);
                if (success)
                {
                    uint16_t fbckValue = GetAnalog((uint8_t)fbckAnalog["port"]);

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

void HcpManager::SetServoMax(std::string name, uint16_t max)
{

    nlohmann::json servos = mapping->GetDevices(Device_Type::SERVO);
    nlohmann::json analogs = mapping->GetDevices(Device_Type::ANALOG);

    if (utils::keyExists(servos, name))
    {
        nlohmann::json servo = servos[name];
        if (utils::keyExists(servo, "feedbackAnalog"))
        {
            string fbckName = servo["feedbackAnalog"];
            if (utils::keyExists(analogs, fbckName))
            {
                nlohmann::json fbckAnalog = analogs[fbckName];

                bool success = SetServoRaw((uint8_t)servo["port"], max);
                if (success)
                {
                    uint16_t fbckValue = GetAnalog((uint8_t)fbckAnalog["port"]);

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

bool HcpManager::SetServoRaw(std::string name, uint16_t onTime)
{
    bool success = false;

    nlohmann::json device = mapping->GetDeviceByName(name, Device_Type::SERVO);

    if (device != nullptr)
    {
        uint8_t port = device["port"];
        success = SetServoRaw(port, onTime);
    }
    else
    {
        Debug::error("name in SetServoRaw not found");
    }
    return success;
}

bool HcpManager::SetServoRaw(uint8_t port, uint16_t onTime)
{
    bool success = false;

    if (!hcpSerial->IsConnected())
    {
        return success;
    }
    if (CheckPort(port, Device_Type::SERVO) && onTime >= 800 && onTime <= 2200)
    {
        //TODO: check if this is okay or if it belongs inside HCP_OK block
        lastServoPosArr[port] = onTime;

        HCP_MSG msg;
        msg.optcode = HCP_SERVO;
        msg.payloadSize = 3;
        msg.payload = new uint8_t[msg.payloadSize];

	    uint16_t dblOnTime = onTime * 2;
	    if (dblOnTime <= SERVO_MAX_ONTIME)
        {
	        uint8_t highOnTime = dblOnTime >> 8;
            if (servoEnabledArr[port])
            {
                highOnTime |= 0x80;
            }

            uint8_t lowOnTime = dblOnTime & 0x00FF;

            msg.payload[0] = port;
            msg.payload[1] = highOnTime;
            msg.payload[2] = lowOnTime;

            serialMtx.lock();
            //fprintf(stderr, "set servo %d to %d\n", port, onTime);
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
                Debug::warning("hcp response message is null");
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
        Debug::info("Servo Port %d not valid in mapping", port);
    }
    return success;
}

bool HcpManager::SetServo(string name, uint8_t percent)
{
    bool success = false;

    nlohmann::json device = mapping->GetDeviceByName(name, Device_Type::SERVO);

    if (device != nullptr)
    {
        success = SetServo(device, percent);
    }
    else
    {
        Debug::error("name in SetServo not found");
    }
    return success;
}

bool HcpManager::SetServo(uint8_t port, uint8_t percent)
{
    bool success = false;

    nlohmann::json device = mapping->GetDeviceByPort(port, Device_Type::SERVO);

    if (device != nullptr)
    {
        success = SetServo(device, percent);
    }
    else
    {
        Debug::error("port in SetServo not found");
    }
    return success;
}

bool HcpManager::SetServo(nlohmann::json device, uint8_t percent)
{
    bool success = false;
    if (device != nullptr)
    {
        uint8_t port = device["port"];
        if (CheckPort(port, Device_Type::SERVO))
        {
            percent = clamp((int)percent, 0, 100);

            int32_t endpoints[2];

            endpoints[0] = device["endpoints"][0];
            endpoints[1] = device["endpoints"][1];

            uint16_t onTime = (((endpoints[1] - endpoints[0]) / 100.0) * percent) + endpoints[0];
            success = SetServoRaw(port, onTime);
        }
        else
        {
            Debug::info("Servo Port %d not valid in mapping", port);
        }
    }
    else
    {
        Debug::error("device in SetServo not found");
    }
    return success;
}

bool HcpManager::SetSupercharge(int8_t setpoint, uint8_t hysteresis)
{
    bool success = false;
	Debug::print("setpoint: %d, hysteresis: %d",setpoint,hysteresis);

    if (!hcpSerial->IsConnected())
    {
        return success;
    }

    HCP_MSG msg;
    msg.optcode = HCP_ST_SUPERCHARGE_SET;
    msg.payloadSize = 2;
    msg.payload = new uint8_t[msg.payloadSize];

    msg.payload[0] = setpoint;
    msg.payload[1] = hysteresis;

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
		Debug::warning("hcp response message is null");
	}
	delete[] msg.payload;

    return success;
}

nlohmann::json HcpManager::GetSupercharge()
{
	nlohmann::json parameters;
	parameters["hysteresis"] = 1;
	parameters["setpoint"] = -1;

	if (!hcpSerial->IsConnected())
    {
        return parameters;
    }

    HCP_MSG msg;
	msg.optcode = HCP_ST_SUPERCHARGE_GET;
	msg.payloadSize = 0;
    msg.payload = nullptr;

	serialMtx.lock();
	//fprintf(stderr, "get analog %d\n", port);
	hcpSerial->Write(msg);
	HCP_MSG* rep = hcpSerial->ReadSync();
	serialMtx.unlock();

	if (rep != nullptr)
	{
		if (rep->optcode == HCP_ST_SUPERCHARGE_REP)
		{
			parameters["setpoint"] = (int8_t)(rep->payload[0]);
			parameters["hysteresis"] = (uint8_t)(rep->payload[1]);
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
		Debug::warning("hcp response message is null");
	}
	delete[] msg.payload;
    
    return parameters;
}

bool HcpManager::SetMotor(uint8_t port, int8_t percent)
{
    return SetMotorRaw(port, Motor_Mode::POWER, percent*10);
}

bool HcpManager::SetMotor(std::string name, Motor_Mode mode, int16_t amount)
{
    bool success = false;

    nlohmann::json device = mapping->GetDeviceByName(name, Device_Type::MOTOR);

    if (device != nullptr)
    {
        uint8_t port = device["port"];
        success = SetMotorRaw(port, mode, amount);
    }
    else
    {
        Debug::error("port in SetMotor not found");
    }
    return success;
}

bool HcpManager::SetMotorRaw(uint8_t port, Motor_Mode mode, int16_t amount)
{
    bool success = false;

    if (!hcpSerial->IsConnected())
    {
        return success;
    }
    if (CheckPort(port, Device_Type::MOTOR))
    {
        amount = clamp((int)amount, -1000, 1000);

        HCP_MSG msg;
        msg.optcode = HCP_MOTOR;
        msg.payloadSize = 4;
        msg.payload = new uint8_t[msg.payloadSize];

        uint8_t highAmount = amount >> 8;

        uint8_t lowAmount = amount & 0x00FF;

        msg.payload[0] = port;
        msg.payload[1] = (uint8_t) mode;
        msg.payload[2] = highAmount;
        msg.payload[3] = lowAmount;

        serialMtx.lock();
        //fprintf(stderr, "set motor %d to %d\n", port, amount);
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
            Debug::warning("hcp response message is null");
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

    nlohmann::json device = mapping->GetDeviceByName(name, Device_Type::DIGITAL_OUT);

    if (device != nullptr)
    {
        uint8_t port = device["port"];
        SetDigitalOutputs(port, enable);
    }
    else
    {
        Debug::error("digital output not found");
    }
    return success;
}

//careful: digitalOut and motor share the same ports
bool HcpManager::SetDigitalOutputs(uint8_t port, bool enable)
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

int32_t *HcpManager::GetLoadCells()
{

    int32_t *value = new int32_t[HCP_THRUST_SENSORS_COUNT];
    std::fill( value, value+HCP_THRUST_SENSORS_COUNT, -1 );

    if (!hcpSerial->IsConnected())
    {
        return value;
    }

    HCP_MSG msg;
    msg.optcode = HCP_ST_THRUST_REQ;
    msg.payloadSize = 0;
    msg.payload = nullptr;



    serialMtx.lock();
    //fprintf(stderr, "get load cells\n");
    hcpSerial->Write(msg);
    HCP_MSG* rep = hcpSerial->ReadSync();
    serialMtx.unlock();

    if (rep != nullptr)
    {
        if (rep->optcode == HCP_ST_THRUST_REP)
        {
            uint8_t signBytes[HCP_THRUST_SENSORS_COUNT] = {0};
            for (int i = 0; i < HCP_THRUST_SENSORS_COUNT; i++)
            {
                if (rep->payload[i*3] >= 0x80)
                {
                    signBytes[i] = 0xFF;
                }

                value[i] = signBytes[i] << 24 | (rep->payload[i*3] << 16) | (rep->payload[(i*3)+1] << 8) | rep->payload[(i*3)+2];

                Debug::info("REP Cell %d: %d", i, value[i]);
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
        Debug::warning("hcp response message is null");
    }
    delete[] msg.payload;

    return value;
}

void HcpManager::TareLoadCells()
{
    nlohmann::json analogs = mapping->GetDevices(Device_Type::ANALOG);
    if (analogs != nullptr)
    {
        for (auto it = analogs.begin(); it != analogs.end(); ++it)
        {
            if (utils::keyExists(it.value(), "loadCells"))
            {
                if (utils::keyExists(it.value(), "map"))
                {
                    Debug::info("mapping thrust values");

                    nlohmann::json loadCells = it.value();
                    nlohmann::json maps = loadCells["map"];

                    if (maps.type() == nlohmann::json::value_t::array)
                    {

                        if (maps.size() > 0)
                        {
                            int32_t* cells = GetLoadCells();

                            for (int i = 0; i < HCP_THRUST_SENSORS_COUNT; i++)
                            {
                                if (utils::keyExists(maps[i], "d") && utils::keyExists(maps[i], "k"))
                                {
                                    std::cout << cells[i] << std::endl;
                                    maps[i]["d"] = -((double)maps[i]["k"] * (double)cells[i]);
                                }
                                else
                                {
                                    Debug::error("no k or d key in load cells map");
                                }
                            }

                            loadCells["map"] = maps;
                            mapping->SetDevice(it.key(), loadCells, Device_Type::ANALOG);
                        }
                        else
                        {
                            Debug::error("map field for load cells is empty");
                        }
                    }
                    else
                    {
                        Debug::error("map field in mapping is not an array");
                    }

                }
                else
                {
                    Debug::error("map field for load cells don't exist");
                }

            }
            else
            {
//                Debug::error("No load cells found");
            }

        }


    }
    else
    {
        Debug::error("No analogs found");
    }
}

double HcpManager::GetAnalog(std::string name)
{
    double value = -1;

    nlohmann::json device = mapping->GetDeviceByName(name, Device_Type::ANALOG);

    if (device != nullptr)
    {
        value = GetAnalog((uint8_t)device["port"]);

        if (utils::keyExists(device, "servo"))
        {
            Debug::info("converting feedback sensor to percentage");
            //get servo of fbck sensor

            string servoName = device["servo"];
            nlohmann::json servo = mapping->GetDeviceByName(servoName, Device_Type::SERVO);

            vector<uint16_t> servoEndpoints = servo["endpoints"];
            vector<uint16_t> fbckEndpoints = servo["feedbackEndpoints"];

            double norm = (((value-fbckEndpoints[0])*1.0) / (fbckEndpoints[1] - fbckEndpoints[0]));

            //convert to us
            //value = ((servoEndpoints[1] - servoEndpoints[0])*norm) + servoEndpoints[0];

            //convert to percentage
            value = norm * 100;
        }
        else if (utils::keyExists(device, "map"))
        {
            Debug::info("mapping sensor value");

            nlohmann::json map = device["map"];

            double before = value;
            value = ((double)value * (double)map["k"]) + (double)map["d"];

            Debug::info("from %d to %d", before, value);
        }
    }
    else
    {
        Debug::error("name in GetAnalog not found");
    }

    return value;
}

int32_t HcpManager::GetAnalog(uint8_t port)
{
    int32_t value = -1;

    if (!hcpSerial->IsConnected())
    {
        return value;
    }
    if (CheckPort(port, Device_Type::ANALOG))
    {
        HCP_MSG msg;
        msg.optcode = HCP_ANALOG_REQ;
        msg.payloadSize = 1;
        msg.payload = new uint8_t[msg.payloadSize];

        msg.payload[0] = port;

        serialMtx.lock();
        //fprintf(stderr, "get analog %d\n", port);
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
            Debug::warning("hcp response message is null");
        }
        delete[] msg.payload;
    }
    else
    {
        Debug::error("Analog Port %d not valid in mapping", port);
    }
    return value;
}

uint8_t HcpManager::GetDigital(std::string name)
{
    uint8_t state = -1;

    nlohmann::json device = mapping->GetDeviceByName(name, Device_Type::DIGITAL);

    if (device != nullptr)
    {
        state = GetDigital((uint8_t)device["port"]);
    }
    else
    {
        Debug::error("name in GetDigital not found");
    }

    return state;
}

uint8_t HcpManager::GetDigital(uint8_t port)
{
    uint8_t state = -1;

    if (!hcpSerial->IsConnected())
    {
        return state;
    }
    if (CheckPort(port, Device_Type::DIGITAL))
    {
        HCP_MSG msg;
        msg.optcode = HCP_DIGITAL_REQ;
        msg.payloadSize = 1;
        msg.payload = new uint8_t[msg.payloadSize];

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
            Debug::warning("hcp response message is null");
        }
        delete[] msg.payload;
    }
    else
    {
        Debug::error("Digital Port %d not valid in mapping", port);
    }
    return state;
}

uint16_t HcpManager::GetBatteryLevel()
{
    return GetAnalog(HCP_ANALOG_SUPPLY_PORT);
}
