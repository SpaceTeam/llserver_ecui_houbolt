//
// Created by Markus on 31.03.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_DIGITALOUT_H
#define LLSERVER_ECUI_HOUBOLT_DIGITALOUT_H

#include <utility>

#include "can/Channel.h"
#include "can/Node.h"
#include "can_houbolt/channels/digital_out_channel_def.h"

class DigitalOut : public Channel, public NonNodeChannel
{
public:
    //TODO: MP check if this is the only and correct way to implement static const with inheritation
    static const std::vector<std::string> states;
    static const std::map<std::string, std::vector<double>> scalingMap;
    static const std::map<DIGITAL_OUT_VARIABLES , std::string> variableMap;

    //-------------------------------RECEIVE Functions-------------------------------//

public:
    DigitalOut(uint8_t channelID, std::string channelName, double sensorScaling, Node *parent);

    std::vector<std::string> GetStates() override;

    //-------------------------------RECEIVE Functions-------------------------------//

    void ProcessCANCommand(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp) override;

    //-------------------------------SEND Functions-------------------------------//

	void SetState(std::vector<double> &params, bool testOnly=false);
	void GetState(std::vector<double> &params, bool testOnly=false);

	void SetDutyCycle(std::vector<double> &params, bool testOnly=false);
	void GetDutyCycle(std::vector<double> &params, bool testOnly=false);

	void SetFrequency(std::vector<double> &params, bool testOnly=false);
	void GetFrequency(std::vector<double> &params, bool testOnly=false);

	void SetRefreshDivider(std::vector<double> &params, bool testOnly=false);
	void GetRefreshDivider(std::vector<double> &params, bool testOnly=false);

	void RequestStatus(std::vector<double> &params, bool testOnly) override;
	void RequestResetSettings(std::vector<double> &params, bool testOnly) override;
};

#endif //LLSERVER_ECUI_HOUBOLT_DIGITALOUT_H
