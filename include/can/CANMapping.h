//
// Created by Markus on 05.04.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_CANMAPPING_H
#define LLSERVER_ECUI_HOUBOLT_CANMAPPING_H

#include "../utility/JSONMapping.h"

class CANMappingObj
{
public:
    std::string stringID;
    double slope;
    double offset;
};

class CANMapping : JSONMapping
{
private:
public:
    CANMapping(std::string mappingPath);
    CANMapping(std::string mappingPath, std::string mappingID);

    CANMappingObj GetNodeObj(uint8_t &nodeID);
    CANMappingObj GetChannelObj(uint8_t &nodeID, uint8_t &channelID);

    void SetChannelObj(uint8_t &nodeID, uint8_t &channelID, CANMappingObj &obj);
};

#endif //LLSERVER_ECUI_HOUBOLT_CANMAPPING_H
