#include "can/CANDriverUDP.h"
#include <utility>
#include <string>
#include <iostream>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/sockios.h>
#include <poll.h>
#include "can_houbolt/can_cmds.h"
#include "utility/utils.h"
#include "utility/Config.h"


CANDriverUDP::CANDriverUDP(std::function<void(uint8_t &, uint32_t &, uint8_t *, uint32_t &, uint64_t &)> onRecvCallback,
									   std::function<void(std::string *)> onErrorCallback) :
	CANDriver(onRecvCallback, onErrorCallback)
{
	std::string ip = std::get<std::string>(Config::getData("LORA/ip"));
    int32_t port = std::get<int>(Config::getData("LORA/port"));
	std::vector<int> canIDSInt= std::get<std::vector<int>>(Config::getData("LORA/canIDs"));
	std::vector<int> canMsgSizesInt = std::get<std::vector<int>>(Config::getData("LORA/canMsgSizes"));
	canIDs = std::vector<uint32_t>(canIDSInt.begin(), canIDSInt.end());
    canMsgSizes = std::vector<uint32_t>(canMsgSizesInt.begin(), canMsgSizesInt.end());

	if (canIDs.size() != canMsgSizes.size())
	{
		throw std::runtime_error("canIDs and canMsgSizes in config don't have the same length");
	}

    socket = new UDPSocket("CAN_UDPSocket", std::bind(&CANDriverUDP::Close, this), ip, port);
    while(socket->Connect()!=0);
    
    connectionActive = true;
    asyncListenThread = new std::thread(&CANDriverUDP::AsyncListen, this);

	Debug::print("CANDriverUDP - CANDriverUDP: init device done.");
}

CANDriverUDP::~CANDriverUDP()
{
	if (connectionActive)
    {
        shallClose = true;
        if (asyncListenThread->joinable())
        {
            asyncListenThread->join();
            delete asyncListenThread;
        }
        
        delete socket;
        connectionActive = false;
    }
}

void CANDriverUDP::AsyncListen()
{
	UDPMessage msg = {0};
    while(!shallClose)
    {
        
        try {
            socket->Recv(&msg);
			CANDriverUDPMessage *udpMessage = (CANDriverUDPMessage *)msg.data;
			uint8_t canBusChannelID = 0;
			uint8_t *payload = udpMessage->payload;
			for (size_t i = 0; i < canIDs.size(); i++)
			{
				onRecvCallback(canBusChannelID, canIDs[i], payload, canMsgSizes[i], udpMessage->timestamp);
				payload += canMsgSizes[i];
			}

            
        } catch (const std::exception& e) {
            Debug::error("CANDriverUDP - AsyncListen: %s", e.what());
            socket->Connect();
        }

        std::this_thread::yield();
    }
}

void CANDriverUDP::SendCANMessage(uint32_t canChannelID, uint32_t canID, uint8_t *payload, uint32_t payloadLength, bool blocking)
{
	uint8_t udpPayload[256];
	UDPMessage msg = {0};
	msg.dataLength = 1+4+2+payloadLength;
	msg.data = udpPayload;
	uint64_t timestamp = utils::getCurrentTimestamp();
	
	msg.data[0] = (uint8_t)MessageType::DATAFRAME;
	msg.data[1] = timestamp >> 24;
	msg.data[2] = timestamp >> 16;
	msg.data[3] = timestamp >> 8;
	msg.data[4] = timestamp;
	msg.data[5] = canID >> 8;
	msg.data[6] = canID;
	std::copy_n(payload, payloadLength, &msg.data[7]);

	socket->Send(&msg);
}


std::map<std::string, bool> CANDriverUDP::GetCANStatusReadable(uint32_t canBusChannelID) // TODO
{
	std::cerr << "CANDriverUDP::GetCANStatusReadable not implemented" << std::endl;
	std::map<std::string, bool> status;
	return status;
}

void CANDriverUDP::Close()
{
	Debug::print("UDPSocket closed");
}