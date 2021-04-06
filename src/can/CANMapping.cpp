//
// Created by Markus on 05.04.21.
//

#include <can/CANMapping.h>

CANMapping::CANMapping(std::string mappingPath) : JSONMapping(mappingPath)
{

}

CANMapping::CANMapping(std::string mappingPath, std::string mappingID) : JSONMapping(mappingPath, mappingID)
{

}

CANMappingObj CANMapping::GetNodeObj(uint8_t &nodeID)
{
    nlohmann::json nodeObj;
    CANMappingObj mappingObj;
    try
    {
        nodeObj = this->mapping[nodeID];
        mappingObj.stringId = nodeObj["stringID"].get<std::string>();
        mappingObj.scaling = nodeObj["scaling"].get<double>();
    }
    catch(std::exception& e)
    {
        Debug::error("Node id or hlid do not exist: %s", e.what());
    }
    return mappingObj;
}

CANMappingObj CANMapping::GetChannelObj(uint8_t &nodeID, uint8_t &channelID)
{
    nlohmann::json channelObj;
    CANMappingObj mappingObj;
    try
    {
        channelObj = this->mapping[nodeID][channelID];
        mappingObj.stringId = channelObj["stringID"].get<std::string>();
        mappingObj.scaling = channelObj["scaling"].get<double>();
    }
    catch(std::exception& e)
    {
        Debug::error("Node id, channel id or hlid do not exist: %s", e.what());
    }
    return mappingObj;
}

