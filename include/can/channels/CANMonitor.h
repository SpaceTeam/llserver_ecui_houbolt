//
// Created by raffael on 31.03.25.
//

#ifndef CANMONITOR_H
#define CANMONITOR_H


#include "can/channels/Channel.h"
#include "../Node.h"
#include "channels/can_monitor_channel_def.hpp"

class CANMonitor : public Channel, public NonNodeChannel
{
    static const std::vector<std::string> states;
    static const std::map<std::string, std::vector<double>> scalingMap;
    static const std::map<CAN_MONITOR_VARIABLES , std::string> variableMap;

public:
    CANMonitor(uint8_t channelID, std::string channelName, Node *parent);

    void GetSensorValue(uint8_t *valuePtr, uint8_t &valueLength, std::vector<std::pair<std::string, double>> &nameValueMap) override;

    //-------------------------------RECEIVE Functions-------------------------------//

    void ProcessCANCommand(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp) override;
    std::vector<std::string> GetStates() override;

    //-------------------------------SEND Functions-------------------------------//

    void SetRefreshDivider(std::vector<double> &params, bool testOnly);
    void GetRefreshDivider(std::vector<double> &params, bool testOnly);

    //-------------------------------Utility Functions-------------------------------//

    void RequestCurrentState() override;

};



#endif //CANMONITOR_H
