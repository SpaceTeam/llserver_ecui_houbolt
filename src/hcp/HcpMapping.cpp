//
// Created by Markus on 2019-10-15.
//

#include "hcp/HcpMapping.h"
#include "utils.h"

using namespace std;

using nlohmann::json;

std::map<Device_Type, std::string> HcpMapping::typeMap = {

        {Device_Type::SERVO, "servo"},
        {Device_Type::MOTOR, "motor"},
        {Device_Type::DIGITAL_OUT, "digitalOut"},
        {Device_Type::ANALOG, "analog"},
        {Device_Type::DIGITAL, "digital"}

};

HcpMapping::HcpMapping(std::string mappingPath)
{
    this->mappingPath = mappingPath;
    LoadMapping();

    //CAREFUL: if device type not in typemap it crashes!!!
    Device_Type currType;
    for (auto typeIt = this->mapping.begin(); typeIt != this->mapping.end(); ++typeIt)
    {
        for (const auto &devTypeItem : typeMap)
        {
            if (devTypeItem.second.compare(typeIt.key()) == 0)
            {
                currType = devTypeItem.first;
            }
        }
        for (auto it = typeIt.value().begin(); it != typeIt.value().end(); ++it)
        {
            typeOfNameMap[it.key()] = currType;
        }
    }
}

HcpMapping::~HcpMapping()
{

}

void HcpMapping::LoadMapping()
{
    Debug::print("loading mapping...");
    this->mapping = nlohmann::json::parse(utils::loadFile(this->mappingPath));
    Debug::print("mapping loaded");
}

void HcpMapping::SaveMapping()
{
    utils::saveFile(this->mappingPath, mapping.dump(4));
    Debug::print("mapping saved");
}

std::string HcpMapping::GetTypeName(Device_Type type)
{
    return this->typeMap[type];
}

//TODO: implement none for type
Device_Type HcpMapping::GetTypeByName(std::string name)
{
    Device_Type type;

    if (typeOfNameMap.find(name) != typeOfNameMap.end())
    {
        type = typeOfNameMap[name];
    }
    else
    {
        Debug::error("Device %s not found", name.c_str());
    }
    return type;
}

nlohmann::json HcpMapping::GetDeviceByName(std::string name, Device_Type type)
{
    nlohmann::json device = nullptr;

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
        Debug::error("HcpMapping is null");
    }
    return device;
}

nlohmann::json HcpMapping::GetDeviceByPort(uint8_t port, Device_Type type)
{
    nlohmann::json device = nullptr;

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
        Debug::error("HcpMapping is null");
    }
    return device;
}

nlohmann::json HcpMapping::GetDevices(Device_Type type)
{
    nlohmann::json devices = nullptr;

    if (mapping != nullptr)
    {
        string typeName = GetTypeName(type);

        devices = mapping[typeName];
    }
    else
    {
        Debug::error("HcpMapping is null");
    }

    return devices;
}

void HcpMapping::SetDevice(std::string name, nlohmann::json devValue, Device_Type type)
{
    string typeName = GetTypeName(type);
    mapping[typeName][name] = devValue;
    SaveMapping();
}

nlohmann::json HcpMapping::GetMapping()
{
    return this->mapping;
}
