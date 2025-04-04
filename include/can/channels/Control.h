//
// Created by Markus on 03.09.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_CONTROL_H
#define LLSERVER_ECUI_HOUBOLT_CONTROL_H

#include "can/channels/Channel.h"
#include "can/Node.h"
#include "can_houbolt/channels/control_channel_def.h"

class Control : public Channel, public NonNodeChannel
{
public:
    //TODO: MP check if this is the only and correct way to implement static const with inheritation
    static const std::vector<std::string> states;
    static const std::map<std::string, std::vector<double>> scalingMap;
    static const std::map<CONTROL_VARIABLES , std::string> variableMap;

    //-------------------------------RECEIVE Functions-------------------------------//

public:
    Control(uint8_t channelID, std::string channelName, std::vector<double> sensorScaling, Node *parent);

    std::vector<std::string> GetStates() override;

    //-------------------------------RECEIVE Functions-------------------------------//

    void ProcessCANCommand(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp) override;

    //-------------------------------SEND Functions-------------------------------//

    void SetEnabled(std::vector<double> &params, bool testOnly);
	void GetEnabled(std::vector<double> &params, bool testOnly);

    void SetTarget(std::vector<double> &params, bool testOnly);
	void GetTarget(std::vector<double> &params, bool testOnly);

    void SetThreshold(std::vector<double> &params, bool testOnly);
	void GetThreshold(std::vector<double> &params, bool testOnly);

    void SetHysteresis(std::vector<double> &params, bool testOnly);
	void GetHysteresis(std::vector<double> &params, bool testOnly);

    void SetActuatorChannelID(std::vector<double> &params, bool testOnly);
	void GetActuatorChannelID(std::vector<double> &params, bool testOnly);

    void SetSensorChannelID(std::vector<double> &params, bool testOnly);
	void GetSensorChannelID(std::vector<double> &params, bool testOnly);

    void SetRefreshDivider(std::vector<double> &params, bool testOnly);
	void GetRefreshDivider(std::vector<double> &params, bool testOnly);

	void RequestStatus(std::vector<double> &params, bool testOnly) override;
	void RequestResetSettings(std::vector<double> &params, bool testOnly) override;

    //-------------------------------Utility Functions-------------------------------//

    void RequestCurrentState() override;
};

#endif //LLSERVER_ECUI_HOUBOLT_CONTROL_H
