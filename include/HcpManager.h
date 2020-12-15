//
// Created by Markus on 2019-09-30.
//

#ifndef TXV_ECUI_LLSERVER_HCPMANAGER_H
#define TXV_ECUI_LLSERVER_HCPMANAGER_H

#include "HcpCommands.h"
#include "Serial.h"
#include "Mapping.h"
#include "Timer.h"
#include "json.hpp"
#include "WatchDog.h"

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
    static Mapping* mapping;
    static uint16 lastServoPosArr[];
    static bool servoEnabledArr[];
    static std::recursive_mutex serialMtx;

    static std::mutex sensorMtx;
    static Timer *sensorTimer;
    static std::map<std::string, double> sensorBuffer;

    static bool CheckPort(uint8 port, Device_Type type);
    static void FetchSensors(uint64 microTime);
    static bool SetServo(nlohmann::json device, uint8 percent);

    HcpManager();

    ~HcpManager();
public:

    static void Init();
    static void Restart();
    static void Destroy();

    static void StartSensorFetch(uint32 sampleRate);
    static void StopSensorFetch();

    static std::vector<std::string> GetAllSensorNames();
    static std::map<std::string, double> GetAllSensors();

    static std::vector<std::string> GetAllOutputNames();

    static nlohmann::json GetAllServoData();

    static bool ExecCommand(std::string name, uint8 percent);

    static bool EnableServo(uint8 port);
    static bool DisableServo(uint8 port);

    static bool EnableAllServos();
    static bool DisableAllServos();

    static void SetServoMin(std::string name, uint16 min);
    static void SetServoMax(std::string name, uint16 max);

    static bool SetServoRaw(std::string port, uint16 onTime);
    static bool SetServoRaw(uint8 port, uint16 onTime);
    static bool SetServo(uint8 port, uint8 percent);
    static bool SetServo(std::string name, uint8 percent);

    static bool SetMotor(uint8 port, int8 percent);
    static bool SetMotorRaw(uint8 port, Motor_Mode mode, int16 amount);
    static bool SetMotor(std::string name, Motor_Mode mode, int16 amount);

    static bool SetDigitalOutputs(uint8 port, bool enable);
    static bool SetDigitalOutputs(std::string name, bool enable);

    //returns exactly 3 int32 values
    static int32 *GetLoadCells();
    static void TareLoadCells();

    static double GetAnalog(std::string name);
    static int32 GetAnalog(uint8 port);

    static uint8 GetDigital(std::string name);
    static uint8 GetDigital(uint8 port);

    static uint16 GetBatteryLevel();


};


#endif //TXV_ECUI_LLSERVER_HCPMANAGER_H
