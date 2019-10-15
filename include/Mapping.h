//
// Created by Markus on 2019-10-15.
//

#ifndef TXV_ECUI_LLSERVER_MAPPING_H
#define TXV_ECUI_LLSERVER_MAPPING_H

#include "config.h"
#include "common.h"

#include "json.hpp"

typedef enum class device_type_e
{
	SERVO,
	MOTOR,
	DIGITAL_OUT,
	ANALOG,
	DIGITAL
} Device_Type;

class Mapping
{

private:

    static std::map<Device_Type, std::string> typeMap;

    std::string mappingPath;
    nlohmann::json mapping = nullptr;


    //TODO: make thread safe
    void LoadMapping();
    void SaveMapping();

    std::string GetTypeName(Device_Type type);

public:

    Mapping(std::string mappingPath);

    ~Mapping();


    nlohmann::json GetDeviceByName(std::string name, Device_Type type);

    //note: use FindObjectByName if possible, it is faster
    nlohmann::json GetDeviceByPort(uint8 port, Device_Type type);

    nlohmann::json GetDevices(Device_Type type);

    void SetDevice(std::string name, nlohmann::json devValue, Device_Type type);

    nlohmann::json GetMapping();

};


#endif //TXV_ECUI_LLSERVER_MAPPING_H
