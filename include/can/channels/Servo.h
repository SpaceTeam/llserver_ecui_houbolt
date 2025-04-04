//
// Created by Markus on 31.08.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_SERVO_H
#define LLSERVER_ECUI_HOUBOLT_SERVO_H

#include "can/channels/Channel.h"
#include "can/Node.h"
#include "can_houbolt/channels/servo_channel_def.h"

class Servo : public Channel, public NonNodeChannel
{
public:
    //TODO: MP check if this is the only and correct way to implement static const with inheritation
    static const std::vector<std::string> states;
    static const std::map<std::string, std::vector<double>> scalingMap;
    static const std::map<SERVO_VARIABLES , std::string> variableMap;

    //-------------------------------RECEIVE Functions-------------------------------//

public:
    Servo(uint8_t channelID, std::string channelName, std::vector<double> sensorScaling, Node *parent);

    std::vector<std::string> GetStates() override;

    //-------------------------------RECEIVE Functions-------------------------------//

    void ProcessCANCommand(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp) override;

    //-------------------------------SEND Functions-------------------------------//

	void SetPosition(std::vector<double> &params, bool testOnly);
	void GetPosition(std::vector<double> &params, bool testOnly);

    void SetPositionRaw(std::vector<double> &params, bool testOnly);
	void GetPositionRaw(std::vector<double> &params, bool testOnly);

    void SetTargetPosition(std::vector<double> &params, bool testOnly);
	void GetTargetPosition(std::vector<double> &params, bool testOnly);

    void SetTargetPressure(std::vector<double> &params, bool testOnly);
	void GetTargetPressure(std::vector<double> &params, bool testOnly);

	void SetMaxSpeed(std::vector<double> &params, bool testOnly);
	void GetMaxSpeed(std::vector<double> &params, bool testOnly);

	void SetMaxAccel(std::vector<double> &params, bool testOnly);
	void GetMaxAccel(std::vector<double> &params, bool testOnly);

    void SetMaxTorque(std::vector<double> &params, bool testOnly);
	void GetMaxTorque(std::vector<double> &params, bool testOnly);

    void SetP(std::vector<double> &params, bool testOnly);
	void GetP(std::vector<double> &params, bool testOnly);

    void SetI(std::vector<double> &params, bool testOnly);
	void GetI(std::vector<double> &params, bool testOnly);

    void SetD(std::vector<double> &params, bool testOnly);
	void GetD(std::vector<double> &params, bool testOnly);

    void SetSensorChannelID(std::vector<double> &params, bool testOnly);
	void GetSensorChannelID(std::vector<double> &params, bool testOnly);

    void SetStartpoint(std::vector<double> &params, bool testOnly);
	void GetStartpoint(std::vector<double> &params, bool testOnly);

    void SetEndpoint(std::vector<double> &params, bool testOnly);
	void GetEndpoint(std::vector<double> &params, bool testOnly);

    void SetPWMEnabled(std::vector<double> &params, bool testOnly);
	void GetPWMEnabled(std::vector<double> &params, bool testOnly);

    void SetRefreshDivider(std::vector<double> &params, bool testOnly);
	void GetRefreshDivider(std::vector<double> &params, bool testOnly);

	void RequestStatus(std::vector<double> &params, bool testOnly) override;
	void RequestResetSettings(std::vector<double> &params, bool testOnly) override;

    void RequestMove(std::vector<double> &params, bool testOnly);

    //-------------------------------Utility Functions-------------------------------//

    void RequestCurrentState() override;
};

#endif //LLSERVER_ECUI_HOUBOLT_SERVO_H
