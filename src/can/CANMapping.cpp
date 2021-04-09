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
        nodeObj = this->mapping[std::to_string(nodeID)];
        Debug::print(nodeObj.dump(4));
        mappingObj.stringID = nodeObj["stringID"];
        mappingObj.scaling = 1;
    }
    catch(std::exception& e)
    {
        Debug::error("Node id %d or stringID do not exist: %s", nodeID, e.what());
    }
    return mappingObj;
}

CANMappingObj CANMapping::GetChannelObj(uint8_t &nodeID, uint8_t &channelID)
{
    nlohmann::json channelObj;
    CANMappingObj mappingObj;
    try
    {
        channelObj = this->mapping[std::to_string(nodeID)][std::to_string(channelID)];
        Debug::print(channelObj.dump(4));
        mappingObj.stringID = channelObj["stringID"];
        mappingObj.scaling = channelObj["scaling"];
    }
    catch(std::exception& e)
    {
        Debug::error("Node id %d, channel id %d or stringID do not exist: %s", nodeID, channelID, e.what());
    }
    return mappingObj;
}

