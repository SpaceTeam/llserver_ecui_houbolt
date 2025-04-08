#ifndef OVERPRESSURE_H
#define OVERPRESSURE_H
#include "Channel.h"
#include "Node.h"
#include "channels/overpressure_channel_def.h"


class Overpressure : public Channel, public NonNodeChannel {
private:
    static const std::vector<std::string> states;
    static const std::map<std::string, std::vector<double>> scalingMap;
    static const std::map<OVERPRESSURE_VARIABLES , std::string> variableMap;

    //-------------------------------RECEIVE Functions-------------------------------//

public:
    Overpressure(uint8_t channelID, std::string channelName, std::vector<double> sensorScaling, Node *parent);

    std::vector<std::string> GetStates() override;

    //-------------------------------RECEIVE Functions-------------------------------//

    void ProcessCANCommand(Can_MessageData_t *canMsg, uint32_t &canMsgLength, uint64_t &timestamp) override;


    //-------------------------------SEND Functions-------------------------------//

    void SetRefreshDivider(std::vector<double> &params, bool testOnly);
    void GetRefreshDivider(std::vector<double> &params, bool testOnly);

    void SetThreshold(std::vector<double> &params, bool testOnly);
    void GetThreshold(std::vector<double> &params, bool testOnly);

    void SetOperatingMode(std::vector<double> &params, bool testOnly);
    void GetOperatingMode(std::vector<double> &params, bool testOnly);

    void RequestStatus(std::vector<double> &params, bool testOnly) override;
    void RequestResetSettings(std::vector<double> &params, bool testOnly) override;

    //-------------------------------Utility Functions-------------------------------//

    void RequestCurrentState() override;
};



#endif //OVERPRESSURE_H
