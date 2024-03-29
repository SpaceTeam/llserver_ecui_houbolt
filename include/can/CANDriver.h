#pragma once

#include <vector>
#include <map>
#include <functional>
#include <string>
#include "common.h"


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
	protected:
		std::function<void(uint8_t &, uint32_t &, uint8_t *, uint32_t &, uint64_t &, CANDriver *driver)> onRecvCallback;
		std::function<void(std::string *)> onErrorCallback;
        std::vector<uint32_t> canBusChannelIDs;

    public:
        CANDriver(std::function<void(uint8_t &, uint32_t &, uint8_t *, uint32_t &, uint64_t &, CANDriver *driver)> onRecvCallback,
                  std::function<void(std::string *)> onErrorCallback);
        virtual ~CANDriver();

        virtual void SendCANMessage(uint32_t canBusChannelID, uint32_t canID, uint8_t *payload, uint32_t payloadLength, bool blocking);

        virtual std::map<std::string, bool> GetCANStatusReadable(uint32_t canChannelID);
};
