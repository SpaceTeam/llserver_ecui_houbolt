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
        Debug::info(nodeObj.dump(4));
        mappingObj.stringID = nodeObj["stringID"];
        mappingObj.slope = 1;
        mappingObj.offset = 0;
    }
    catch(std::exception& e)
    {
        Debug::error("Node id %d or stringID do not exist: %s", nodeID, e.what());
        throw std::runtime_error("CANMapping - GetNodeObj: failed");
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
        Debug::info(channelObj.dump(4));
        mappingObj.stringID = channelObj["stringID"];
        try
        {
        	mappingObj.slope = channelObj["slope"];
        }
        catch(std::exception& e)
		{
        	Debug::warning("For channel %s no slope is specified, using 1.0", mappingObj.stringID.c_str());
        	mappingObj.slope = 1.0;
		}
        try
		{
        	mappingObj.offset = channelObj["offset"];
		}
		catch(std::exception& e)
		{
			Debug::warning("For channel %s no offset is specified, using 0.0", mappingObj.stringID.c_str());
			mappingObj.offset = 0.0;
		}
    }
    catch(std::exception& e)
    {
        Debug::error("Node id %d, channel id %d or stringID do not exist: %s", nodeID, channelID, e.what());
        throw std::runtime_error("CANMapping - GetChannelObj: failed");
    }
    return mappingObj;
}

