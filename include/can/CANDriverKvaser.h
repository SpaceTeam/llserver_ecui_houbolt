#pragma once

#ifndef NO_CANLIB

#include "CANDriver.h"


#include <vector>
#include <map>
#include <functional>
#include <string>
#include <canlib.h>
#include <readerwriterqueue.h>
#include "utility/Config.h"
#include "CanKvaserReceiveThread.h"
#include "CommonKvaser.h"


class CANDriverKvaser : public CANDriver
{
    private:
        std::map<uint32_t, canHandle> canHandlesMap = std::map<uint32_t, canHandle>();

		std::map<uint32_t, CANParams> arbitrationParamsMap = std::map<uint32_t, CANParams>();
		std::map<uint32_t, CANParams> dataParamsMap = std::map<uint32_t, CANParams>();

        std::shared_ptr<::moodycamel::ReaderWriterQueue<std::unique_ptr<RawKvaserMessage>>> receivedMessagesQueue;
        std::unique_ptr<CanKvaserReceiveThread> receiveThread;

        uint64_t blockingTimeout;

        static void OnCANCallback(int handle, void *driver, unsigned int event);
        std::string CANError(canStatus status);
        canStatus InitializeCANChannel(uint32_t canChannelID);

    public:
        CANDriverKvaser(canRecvCallback_t onRecvCallback,
						std::function<void(std::string *)> onErrorCallback, std::vector<uint32_t> &canBusChannelIDs, Config &config);
        ~CANDriverKvaser();

        void SendCANMessage(uint32_t canBusChannelID, uint32_t canID, uint8_t *payload, uint32_t payloadLength, bool blocking);

        std::map<std::string, bool> GetCANStatusReadable(uint32_t canChannelID);
};

#endif //NO_CANLIB
