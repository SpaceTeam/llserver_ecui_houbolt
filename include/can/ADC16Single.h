//
// Created by Markus on 31.03.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_ADC16SINGLE_H
#define LLSERVER_ECUI_HOUBOLT_ADC16SINGLE_H

#include "common.h"

#include "can/Channel.h"
#include "can/Node.h"
#include "can_houbolt/channels/adc16_single_channel_def.h"

#include <map>
#include <utility>

class ADC16Single : public Channel, public NonNodeChannel
{
private:
    //TODO: MP check if this is the only and correct way to implement static const with inheritation
    static const std::vector<std::string> states;
    static const std::map<std::string, std::vector<double>> scalingMap;
    static const std::map<ADC16_SINGLE_VARIABLES , std::string> variableMap;

    //-------------------------------RECEIVE Functions-------------------------------//

    void CalibrateResponse(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp);

public:
    ADC16Single(uint8_t channelID, std::string channelName, std::vector<double> sensorScaling, Node *parent);

    std::vector<std::string> GetStates() override;

    //-------------------------------RECEIVE Functions-------------------------------//

    void ProcessCANCommand(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp) override;

    //-------------------------------SEND Functions-------------------------------//

    
    void SetMeasurement(std::vector<double> &params, bool testOnly);
	void GetMeasurement(std::vector<double> &params, bool testOnly);

    void SetRefreshDivider(std::vector<double> &params, bool testOnly);
	void GetRefreshDivider(std::vector<double> &params, bool testOnly);

    void SetData(std::vector<double> &params, bool testOnly);
	void GetData(std::vector<double> &params, bool testOnly);

	void RequestCalibrate(std::vector<double> &params, bool testOnly);

	void RequestStatus(std::vector<double> &params, bool testOnly) override;
	void RequestResetSettings(std::vector<double> &params, bool testOnly) override;

    //-------------------------------Utility Functions-------------------------------//

    void RequestCurrentState() override;
};

#endif //LLSERVER_ECUI_HOUBOLT_ADC16SINGLE_H
