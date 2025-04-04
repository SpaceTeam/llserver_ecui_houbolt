//
// Created by Markus on 03.09.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_PNEUMATIC_VALVE_H
#define LLSERVER_ECUI_HOUBOLT_PNEUMATIC_VALVE_H

#include "can/Channel.h"
#include "can/Node.h"
#include "can_houbolt/channels/pneumatic_valve_channel_def.h"

class PneumaticValve : public Channel, public NonNodeChannel
{
public:
    //TODO: MP check if this is the only and correct way to implement static const with inheritation
    static const std::vector<std::string> states;
    static const std::map<std::string, std::vector<double>> scalingMap;
    static const std::map<PNEUMATIC_VALVE_VARIABLES , std::string> variableMap;

    //-------------------------------RECEIVE Functions-------------------------------//

public:
    PneumaticValve(uint8_t channelID, std::string channelName, std::vector<double> sensorScaling, Node *parent);

    std::vector<std::string> GetStates() override;

    //-------------------------------RECEIVE Functions-------------------------------//

    void ProcessCANCommand(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp) override;

    //-------------------------------SEND Functions-------------------------------//

    void SetEnabled(std::vector<double> &params, bool testOnly);
	void GetEnabled(std::vector<double> &params, bool testOnly);

	void SetPosition(std::vector<double> &params, bool testOnly);
	void GetPosition(std::vector<double> &params, bool testOnly);

    void SetTargetPosition(std::vector<double> &params, bool testOnly);
	void GetTargetPosition(std::vector<double> &params, bool testOnly);

    void SetThreshold(std::vector<double> &params, bool testOnly);
	void GetThreshold(std::vector<double> &params, bool testOnly);

    void SetHysteresis(std::vector<double> &params, bool testOnly);
	void GetHysteresis(std::vector<double> &params, bool testOnly);

    void SetOnChannelID(std::vector<double> &params, bool testOnly);
	void GetOnChannelID(std::vector<double> &params, bool testOnly);

    void SetOffChannelID(std::vector<double> &params, bool testOnly);
	void GetOffChannelID(std::vector<double> &params, bool testOnly);

    void SetPosChannelID(std::vector<double> &params, bool testOnly);
	void GetPosChannelID(std::vector<double> &params, bool testOnly);

    void SetRefreshDivider(std::vector<double> &params, bool testOnly);
	void GetRefreshDivider(std::vector<double> &params, bool testOnly);

	void RequestStatus(std::vector<double> &params, bool testOnly) override;
	void RequestResetSettings(std::vector<double> &params, bool testOnly) override;

    //-------------------------------Utility Functions-------------------------------//

    void RequestCurrentState() override;
};

#endif //LLSERVER_ECUI_HOUBOLT_PNEUMATIC_VALVE_H
