//
// Created by Markus on 2019-10-15.
//

#include "Mapping.h"
#include "utils.h"

using namespace std;

std::map<Device_Type, std::string> Mapping::typeMap = {

        {Device_Type::SERVO, "servo"},
        {Device_Type::MOTOR, "motor"},
        {Device_Type::DIGITAL_OUT, "digitalOut"},
        {Device_Type::ANALOG, "analog"},
        {Device_Type::DIGITAL, "digital"}

};

Mapping::Mapping(std::string mappingPath)
{
    this->mappingPath = mappingPath;
    LoadMapping();
}

Mapping::~Mapping()
{

}

void Mapping::LoadMapping()
{
    Debug::info("loading mapping...");
    this->mapping = json::parse(utils::loadFile(this->mappingPath));
    Debug::info("mapping loaded");
}

void Mapping::SaveMapping()
{
    utils::saveFile(this->mappingPath, mapping.dump(4));
    Debug::info("mapping saved");
}

std::string Mapping::GetTypeName(Device_Type type)
{
    return this->typeMap[type];
}

json Mapping::GetDeviceByName(std::string name, Device_Type type)
{
    json device = nullptr;

    if (mapping != nullptr)
    {
        string typeName = GetTypeName(type);

        if (utils::keyExists(mapping[typeName], name))
        {
            device = mapping[typeName][name];
        }
        else
        {
            Debug::warning("Object: " + name + "not found");
        }
    }
    else
    {
        Debug::error("Mapping is null");
    }
    return device;
}

json Mapping::GetDeviceByPort(uint8 port, Device_Type type)
{
    json device = nullptr;

    if (mapping != nullptr)
    {
        string typeName = GetTypeName(type);

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

nlohmann::json Mapping::GetDevices(Device_Type type)
{
    json devices = nullptr;

    if (mapping != nullptr)
    {
        string typeName = GetTypeName(type);

        devices = mapping[typeName];
    }
    else
    {
        Debug::error("Mapping is null");
    }

    return devices;
}

void Mapping::SetDevice(std::string name, nlohmann::json devValue, Device_Type type)
{
    string typeName = GetTypeName(type);
    mapping[typeName][name] = devValue;
    SaveMapping();
}

nlohmann::json Mapping::GetMapping()
{
    return this->mapping;
}