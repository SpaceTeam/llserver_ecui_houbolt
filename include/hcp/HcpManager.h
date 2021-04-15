//
// Created by Markus on 2019-09-30.
//

#ifndef TXV_ECUI_LLSERVER_HCPMANAGER_H
#define TXV_ECUI_LLSERVER_HCPMANAGER_H

#include "hcp/HcpCommands.h"
#include "drivers/Serial.h"
#include "hcp/HcpMapping.h"
#include "utility/Timer.h"
#include "utility/json.hpp"
#include "utility/WatchDog.h"

typedef enum class battery_status_e
{
	BATTERY_STATUS_EMPTY,
	BATTERY_STATUS_LOW,
	BATTERY_STATUS_OK
} Battery_Status;

typedef enum class motor_mode_e
{
	POWER = HCP_MOTOR_MODE_POWER,
	BRAKE = HCP_MOTOR_MODE_BRAKE,
	VELOCITY = HCP_MOTOR_MODE_VELOCITY
} Motor_Mode;

class HcpManager
{

private:
    static Serial* hcpSerial;
    static HcpMapping* mapping;
    static uint16_t lastServoPosArr[];
    static bool servoEnabledArr[];
    static std::recursive_mutex serialMtx;

    static std::mutex sensorMtx;
    static Timer *sensorTimer;
    static std::map<std::string, double> sensorBuffer;

    static bool CheckPort(uint8_t port, Device_Type type);
    static void FetchSensors(uint64_t microTime);
    static bool SetServo(nlohmann::json device, uint8_t percent);

    HcpManager();

    ~HcpManager();
public:

    static void Init();
    static void Restart();
    static void Destroy();

    static void StartSensorFetch(uint32_t sampleRate);
    static void StopSensorFetch();

    static std::vector<std::string> GetAllSensorNames();
    static std::map<std::string, double> GetAllSensors();

    static std::vector<std::string> GetAllOutputNames();

    static nlohmann::json GetAllServoData();

    static bool ExecCommand(std::string name, uint8_t percent);

    static bool EnableServo(uint8_t port);
    static bool DisableServo(uint8_t port);

    static bool EnableAllServos();
    static bool DisableAllServos();

    static void SetServoMin(std::string name, uint16_t min);
    static void SetServoMax(std::string name, uint16_t max);

    static bool SetServoRaw(std::string port, uint16_t onTime);
    static bool SetServoRaw(uint8_t port, uint16_t onTime);
    static bool SetServo(uint8_t port, uint8_t percent);
    static bool SetServo(std::string name, uint8_t percent);

	static bool SetSupercharge(int8_t setpoint, uint8_t hysteresis);
	static nlohmann::json GetSupercharge();
	
    static bool SetMotor(uint8_t port, int8_t percent);
    static bool SetMotorRaw(uint8_t port, Motor_Mode mode, int16_t amount);
    static bool SetMotor(std::string name, Motor_Mode mode, int16_t amount);

    static bool SetDigitalOutputs(uint8_t port, bool enable);
    static bool SetDigitalOutputs(std::string name, bool enable);

    //returns exactly 3 int32_t values
    static int32_t *GetLoadCells();
    static void TareLoadCells();

    static double GetAnalog(std::string name);
    static int32_t GetAnalog(uint8_t port);

    static uint8_t GetDigital(std::string name);
    static uint8_t GetDigital(uint8_t port);

    static uint16_t GetBatteryLevel();


};


#endif //TXV_ECUI_LLSERVER_HCPMANAGER_H
