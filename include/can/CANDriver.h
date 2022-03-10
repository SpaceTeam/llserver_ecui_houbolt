//
// Created by Markus on 31.03.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_CANDRIVER_H
#define LLSERVER_ECUI_HOUBOLT_CANDRIVER_H

#include "common.h"

#include <vector>
#include <map>
#include <functional>
#include <string>
#include <canlib.h>
//#include <bus_params_tq.h>

#define CAN_CHANNELS 3

typedef struct
{
    int64_t bitrate;
    int32_t timeSegment1;
    int32_t timeSeqment2;
    int32_t syncJumpWidth;
    int32_t noSamplingPoints; //unused for can fd data params
} CANParams;

const std::map<uint32_t, uint32_t> dlcToCANFDMsgLength = {
        {0, 0},
        {1, 1},
        {2, 2},
        {3, 3},
        {4, 4},
        {5, 5},
        {6, 6},
        {7, 7},
        {8, 8},
        {9, 12},
        {10, 16},
        {11, 20},
        {12, 24},
        {13, 32},
        {14, 48},
        {15, 64}
};

class CANDriver
{
    private:
        std::function<void(uint8_t &, uint32_t &, uint8_t *, uint32_t &, uint64_t &)> onRecvCallback;
        std::function<void(uint8_t &, uint32_t &, uint8_t *, uint32_t &, uint64_t &)> seqRecvCallback;
        std::function<void(std::string *)> onErrorCallback;

        CANParams arbitrationParams;
        CANParams dataParams;
        canHandle canHandles[CAN_CHANNELS];

        static void OnCANCallback(int handle, void *driver, unsigned int event);
        std::string CANError(canStatus status);
        canStatus InitializeCANChannel(uint32_t canChannelID);

    public:
        CANDriver(std::function<void(uint8_t &, uint32_t &, uint8_t *, uint32_t &, uint64_t &)> onInitRecvCallback,
                  std::function<void(uint8_t &, uint32_t &, uint8_t *, uint32_t &, uint64_t &)> onRecvCallback,
                  std::function<void(std::string *)> onErrorCallback, CANParams arbitrationParams, CANParams dataParams);
        ~CANDriver();

        //Tells the can driver that initialization is done and canlib callback gets rerouted from initrecvcallback to recvcallback
        void InitDone(void);
        void SendCANMessage(uint32_t canBusChannelID, uint32_t canID, uint8_t *payload, uint32_t payloadLength);

        std::map<std::string, bool> GetCANStatusReadable(uint32_t canChannelID);

};

#endif //LLSERVER_ECUI_HOUBOLT_CANDRIVER_H
