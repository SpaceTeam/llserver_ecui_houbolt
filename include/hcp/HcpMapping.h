//
// Created by Markus on 2019-10-15.
//

#ifndef TXV_ECUI_LLSERVER_HCP_MAPPING_H
#define TXV_ECUI_LLSERVER_HCP_MAPPING_H

#include "common.h"

#include <map>

#include "utility/json.hpp"

typedef enum class device_type_e
{
	SERVO,
	MOTOR,
	DIGITAL_OUT,
	ANALOG,
	DIGITAL
} Device_Type;

class HcpMapping
{

private:

    static std::map<Device_Type, std::string> typeMap;

    //WARNING: DO NOT USE typeOfNameMap unless each name is unique
    std::map<std::string, Device_Type> typeOfNameMap;

    std::string mappingPath;
    nlohmann::json mapping = nullptr;


    //TODO: make thread safe
    void LoadMapping();
    void SaveMapping();

    std::string GetTypeName(Device_Type type);

public:

    HcpMapping(std::string mappingPath);

    ~HcpMapping();

    //WARNING: DO NOT USE typeOfNameMap unless each name is unique
    Device_Type GetTypeByName(std::string name);

    nlohmann::json GetDeviceByName(std::string name, Device_Type type);

    //note: use FindObjectByName if possible, it is faster
    nlohmann::json GetDeviceByPort(uint8_t port, Device_Type type);

    nlohmann::json GetDevices(Device_Type type);

    void SetDevice(std::string name, nlohmann::json devValue, Device_Type type);

    nlohmann::json GetMapping();

};


#endif //TXV_ECUI_LLSERVER_HCP_MAPPING_H
