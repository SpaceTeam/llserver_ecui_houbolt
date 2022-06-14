#pragma once

#include <vector>
#include <map>
#include <functional>
#include <string>
#include <thread>
#include "common.h"
#include "CANDriver.h"


class CANDriverSocketCAN : public CANDriver
{
    private:
		void receiveLoop();
		bool done = false;
		std::thread* receiveThread;

		std::vector<std::string> canDevices;
		int canSocket;

    public:
        CANDriverSocketCAN(std::function<void(uint8_t &, uint32_t &, uint8_t *, uint32_t &, uint64_t &, CANDriver *driver)> onRecvCallback,
					   	   std::function<void(std::string *)> onErrorCallback);
        ~CANDriverSocketCAN();

        void SendCANMessage(uint32_t canBusChannelID, uint32_t canID, uint8_t *payload, uint32_t payloadLength, bool blocking);

        std::map<std::string, bool> GetCANStatusReadable(uint32_t canChannelID);
};
