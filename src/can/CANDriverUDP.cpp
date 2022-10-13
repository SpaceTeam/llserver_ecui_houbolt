#include "can/CANDriverUDP.h"
#include "can_houbolt/can_cmds.h"
#include <string>
#include <sys/socket.h>
#include <net/if.h>
#include "can_houbolt/can_cmds.h"
#include "can_houbolt/channels/generic_channel_def.h"
#include "utility/utils.h"
#include "utility/Config.h"
#include "can/Node.h"


CANDriverUDP::CANDriverUDP(std::function<void(uint8_t &, uint32_t &, uint8_t *, uint32_t &, uint64_t &, CANDriver *driver)> onRecvCallback,
									   std::function<void(std::string *)> onErrorCallback) :
	CANDriver(onRecvCallback, onErrorCallback)
{
	std::string sendIP = std::get<std::string>(Config::getData("LORA/ip"));
    int32_t sendPort = std::get<int>(Config::getData("LORA/port"));
	std::vector<int> canIDSInt= std::get<std::vector<int>>(Config::getData("LORA/nodeIDs"));
	std::vector<int> canMsgSizesInt = std::get<std::vector<int>>(Config::getData("LORA/canMsgSizes"));

	for (auto &size : canMsgSizesInt)
	{
		if (size <= 0)
		{
			throw std::runtime_error("canMsgSizes entry must not be 0 or negative");
		}
		totalRequiredMsgPayloadSize += CAN_MSG_HEADER_SIZE + size;
	}

	nodeIDs = std::vector<uint32_t>(canIDSInt.begin(), canIDSInt.end());
    canMsgSizes = std::vector<uint32_t>(canMsgSizesInt.begin(), canMsgSizesInt.end());

	if (nodeIDs.size() != canMsgSizes.size())
	{
		throw std::runtime_error("nodeIDs and canMsgSizes in config don't have the same length");
	}

    socket = new UDPSocket("CAN_UDPSocket", std::bind(&CANDriverUDP::Close, this), sendIP, sendPort);
    while(socket->Connect()!=0);
    
    connectionActive = true;
	shallClose = false;
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

			if (msg.dataLength <= 0)
			{
				Debug::error("CANDriverUDP - AsyncListen: msg length smaller than one, ignoring message...");
				continue;
			}
			
			uint8_t canBusChannelID = 0;
			uint8_t *payload = msg.data;
			uint64_t timestamp = utils::getCurrentTimestamp();
			if (msg.dataLength != totalRequiredMsgPayloadSize + MSG_HEADER_SIZE)
			{
				Debug::error("CANDriverUDP - AsyncListen: msg length is too short or too long\n\t\trequired: %d, actual %d\n\t\tignoring message...", totalRequiredMsgPayloadSize+MSG_HEADER_SIZE, msg.dataLength);
				continue;
			}

			for (size_t i=0; i < msg.dataLength; ++i)
			{
					if (i % 8 == 0)
					{
							printf("\n%zd\t", i);
					}
					printf("0x%02x ", msg.data[i]);
			}
			printf("\n");

			//create canid
			Can_MessageId_t canID = {0};
			canID.info.direction = NODE2MASTER_DIRECTION;
			canID.info.priority = STANDARD_PRIORITY;
			canID.info.special_cmd = STANDARD_SPECIAL_CMD;

			Can_MessageData_t command = {0};
			command.bit.info.buffer = DIRECT_BUFFER;
			command.bit.info.channel_id = GENERIC_CHANNEL_ID;
			command.bit.cmd_id = GENERIC_RES_DATA;

			for (size_t i = 0; i < nodeIDs.size(); i++)
			{
				switch (payload[0])
				{
					case (uint8_t)CanMessageOption::USED:
						{
							canID.info.node_id = nodeIDs[i];
							std::memcpy(command.bit.data.uint8, &payload[CAN_MSG_HEADER_SIZE], canMsgSizes[i]);
							onRecvCallback(canBusChannelID, canID.uint32, command.uint8, canMsgSizes[i], timestamp, this);
							//Node::FlushLogger();
						}
						break;
					case (uint8_t)CanMessageOption::EMPTY:
						Debug::info("CANDriverUDP - AsyncListen: empty can message recieved, ignoring block...");
						break;
					default:
						Debug::error("CANDriverUDP - AsyncListen: CanMessageOption 0x%02x not recognized, ignoring can msg", payload[0]);
						continue;
				}
				payload += CAN_MSG_HEADER_SIZE + canMsgSizes[i];
				
			}

            
        } catch (const std::exception& e) {
            Debug::error("CANDriverUDP - AsyncListen: %s", e.what());
        }

    }
}

void CANDriverUDP::SendCANMessage(uint32_t canChannelID, uint32_t canID, uint8_t *payload, uint32_t payloadLength, bool blocking)
{
	uint8_t udpPayload[256] = {0};
	UDPMessage msg = {0};
	msg.dataLength = MSG_HEADER_SIZE+totalRequiredMsgPayloadSize;
	msg.data = udpPayload;

	Can_MessageId_t *canIDStruct = (Can_MessageId_t *)&canID;
	
	//node id byte is used instead of can option
	msg.data[0] = canIDStruct->info.node_id;
	std::copy_n(payload, payloadLength, &msg.data[1]);

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