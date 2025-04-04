//
// Created by Markus on 03.09.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_PI_CONTROL_H
#define LLSERVER_ECUI_HOUBOLT_PI_CONTROL_H

#include "can/Channel.h"
#include "can/Node.h"
#include "can_houbolt/channels/pi_control_channel_def.h"

class PIControl : public Channel, public NonNodeChannel
{
public:
    //TODO: MP check if this is the only and correct way to implement static const with inheritation
    static const std::vector<std::string> states;
    static const std::map<std::string, std::vector<double>> scalingMap;
    static const std::map<PI_CONTROL_VARIABLES, std::string> variableMap;

    //-------------------------------RECEIVE Functions-------------------------------//

public:
    PIControl(uint8_t channelID, std::string channelName, std::vector<double> sensorScaling, Node *parent);

    std::vector<std::string> GetStates() override;

    //-------------------------------RECEIVE Functions-------------------------------//

    void ProcessCANCommand(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp) override;

    //-------------------------------SEND Functions-------------------------------//

    void SetEnabled(std::vector<double> &params, bool testOnly);
	void GetEnabled(std::vector<double> &params, bool testOnly);

    void SetTarget(std::vector<double> &params, bool testOnly);
	void GetTarget(std::vector<double> &params, bool testOnly);

    void SetP_POS(std::vector<double> &params, bool testOnly);
	void GetP_POS(std::vector<double> &params, bool testOnly);

    void SetI_POS(std::vector<double> &params, bool testOnly);
	void GetI_POS(std::vector<double> &params, bool testOnly);

    void SetP_NEG(std::vector<double> &params, bool testOnly);
	void GetP_NEG(std::vector<double> &params, bool testOnly);

    void SetI_NEG(std::vector<double> &params, bool testOnly);
	void GetI_NEG(std::vector<double> &params, bool testOnly);

    void SetSensorSlope(std::vector<double> &params, bool testOnly);
	void GetSensorSlope(std::vector<double> &params, bool testOnly);

    void SetSensorOffset(std::vector<double> &params, bool testOnly);
	void GetSensorOffset(std::vector<double> &params, bool testOnly);

    void SetOperatingPoint(std::vector<double> &params, bool testOnly);
	void GetOperatingPoint(std::vector<double> &params, bool testOnly);

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

#endif //LLSERVER_ECUI_HOUBOLT_PI_CONTROL_H
