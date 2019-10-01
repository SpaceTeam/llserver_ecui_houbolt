//
// Created by Markus on 2019-09-30.
//

#ifndef TXV_ECUI_LLSERVER_HCPMANAGER_H
#define TXV_ECUI_LLSERVER_HCPMANAGER_H

#include "config.h"

#include "Serial.h"
#include "json.hpp"

typedef enum class battery_status_e
{
	BATTERY_STATUS_EMPTY,
	BATTERY_STATUS_LOW,
	BATTERY_STATUS_OK
} Battery_Status;

typedef enum class device_type_e
{
	SERVO,
	ANALOG,
	DIGITAL
} Device_Type;

class HcpManager
{

private:
    static Serial* hcpSerial;
    static json mapping;
    static uint8 lastServoPosArr[];
    static bool servoEnabledArr[];

    static void LoadMapping();
    static void SaveMapping();

    static bool CheckPort(uint8 port, Device_Type type);
    static nlohmann::json FindObjectByName(std::string name, Device_Type type);
    static nlohmann::json FindObjectByPort(uint8 port, Device_Type type);

    static bool SetServo(json device, uint8 percent);

    HcpManager();

    ~HcpManager();
public:

    static void init();
    static void restart();

    static bool ExecCommand(std::string name, uint8 percent);

    static bool EnableServo(uint8 port);
    static bool DisableServo(uint8 port);
    static bool SetServoRaw(uint8 port, uint16 onTime);
    static bool SetServo(uint8 port, uint8 percent);
    static bool SetServo(std::string name, uint8 percent);

    static uint16 GetAnalog(std::string name);
    static uint16 GetAnalog(uint8 port);

    static uint8 GetDigital(std::string name);
    static uint8 GetDigital(uint8 port);

    static uint16 GetBatteryLevel();


};


#endif //TXV_ECUI_LLSERVER_HCPMANAGER_H
