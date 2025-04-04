//
// Created by Markus on 31.08.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_ROCKET_H
#define LLSERVER_ECUI_HOUBOLT_ROCKET_H

#include "can/channels/Channel.h"
#include "can/Node.h"
#include "can_houbolt/channels/rocket_channel_def.h"

class Rocket : public Channel, public NonNodeChannel
{
private:
    //TODO: MP check if this is the only and correct way to implement static const with inheritation
    static const std::vector<std::string> states;
    static const std::map<std::string, std::vector<double>> scalingMap;
    static const std::map<ROCKET_VARIABLES , std::string> variableMap;

    //-------------------------------RECEIVE Functions-------------------------------//

    void RocketStateResponse(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp);
    void InternalControlResponse(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp);
    void AbortResponse(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp);
    void EndOfFlightResponse(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp);
    void AutoCheckResponse(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp);

public:
    Rocket(uint8_t channelID, std::string channelName, std::vector<double> sensorScaling, Node *parent);

    std::vector<std::string> GetStates() override;

    //-------------------------------RECEIVE Functions-------------------------------//

    void ProcessCANCommand(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp) override;

    //-------------------------------SEND Functions-------------------------------//

	void SetMinimumChamberPressure(std::vector<double> &params, bool testOnly);
	void GetMinimumChamberPressure(std::vector<double> &params, bool testOnly);

    void SetMinimumFuelPressure(std::vector<double> &params, bool testOnly);
	void GetMinimumFuelPressure(std::vector<double> &params, bool testOnly);

	void SetMinimumOxPressure(std::vector<double> &params, bool testOnly);
	void GetMinimumOxPressure(std::vector<double> &params, bool testOnly);

	void SetHolddownTimeout(std::vector<double> &params, bool testOnly);
	void GetHolddownTimeout(std::vector<double> &params, bool testOnly);

    void SetStateRefreshDivider(std::vector<double> &params, bool testOnly);
	void GetStateRefreshDivider(std::vector<double> &params, bool testOnly);

    void SetRocketState(std::vector<double> &params, bool testOnly);
    void GetRocketState(std::vector<double> &params, bool testOnly);
    
    void RequestInternalControl(std::vector<double> &params, bool testOnly);
    void RequestAbort(std::vector<double> &params, bool testOnly);
    void RequestEndOfFlight(std::vector<double> &params, bool testOnly);
    void RequestAutoCheck(std::vector<double> &params, bool testOnly);

	void RequestStatus(std::vector<double> &params, bool testOnly) override;
	void RequestResetSettings(std::vector<double> &params, bool testOnly) override;

    //-------------------------------Utility Functions-------------------------------//

    void RequestCurrentState() override;

};

#endif //LLSERVER_ECUI_HOUBOLT_ROCKET_H
