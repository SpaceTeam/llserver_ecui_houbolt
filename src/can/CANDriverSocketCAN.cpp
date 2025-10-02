	#include "can/CANDriverSocketCAN.h"
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


CANDriverSocketCAN::CANDriverSocketCAN(canRecvCallback_t onRecvCallback,
									   std::function<void(std::string *)> onErrorCallback, Config &config) :
	CANDriver(onRecvCallback, onErrorCallback)
{
	canDevices = (std::vector<std::string>)config["/CAN/DEVICE"];

	// create can socket
	canSocket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if(canSocket < 0) throw std::runtime_error("CAN socket creation failed");

	//switch to FD mode
	int enable_canfd = 1;
	if(setsockopt(canSocket, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &enable_canfd, sizeof(enable_canfd)))
		throw std::runtime_error("CAN switching to FD mode failed");

	// find interface index of our can device
	struct ifreq ifr;
	strcpy(ifr.ifr_name, canDevices[0].c_str());
	ioctl(canSocket, SIOCGIFINDEX, &ifr);

	// TODO: set bit timing (e.g. using libsocketcan when it starts supporting CAN FD), currently must be set using ip link

	// add filter to ignore messages with direction=0
	struct can_filter rfilter[1];
	rfilter[0].can_id   = 0x01;
	rfilter[0].can_mask = 0x01;
	if(setsockopt(canSocket, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter)))
		throw std::runtime_error("CAN adding filter failed");

	// TODO: if using multiple interfaces, the interface index can be set to zero to bind to all (block above not needed). recvfrom and sendto should be used in that case
	// bind socket to candevice
	struct sockaddr_can addr;
	memset(&addr, 0, sizeof(addr));
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;
	if(bind(canSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0) throw std::runtime_error("CAN socket bind failed");

	// start thread that handles incoming messages
	receiveThread = new std::thread(&CANDriverSocketCAN::receiveLoop, this);

	Debug::print("CANDriverSocketCAN: init with device %s done.", canDevices[0].c_str());
}

CANDriverSocketCAN::~CANDriverSocketCAN()
{
	done = true;
	if(receiveThread->joinable()) receiveThread->join();
	else Debug::warning("receiveThread was not joinable.");
	delete receiveThread;
	close(canSocket);
}


void CANDriverSocketCAN::SendCANMessage(uint32_t canChannelID, uint32_t canID, uint8_t *payload, uint32_t payloadLength, bool blocking)
{
	if(canChannelID > 0) return; // TODO: support multiple devices, use canChannelID

    if(payloadLength > MAX_DATA_SIZE) throw std::runtime_error("CANDriver - SendCANMessage: payload length " + std::to_string(payloadLength) + " exceeds supported can fd msg data size " + std::to_string(MAX_DATA_SIZE));

	struct canfd_frame frame;
	frame.can_id = canID & 0x7FF; // remove flags, use 0x1FFFFFFF to support extended IDs
	frame.can_id |= 0; // no flags, use 0x80000000 for extended ID
	frame.len = payloadLength;
	std::memcpy(frame.data, payload, payloadLength);

	if(write(canSocket, &frame, sizeof(struct canfd_frame)) != sizeof(struct canfd_frame))
	{
		Debug::print("Errno: 0x%x", errno);
		throw std::runtime_error("CAN write failed");
	}

	//TODO wait for completion if blocking. is this possible?
}


std::map<std::string, bool> CANDriverSocketCAN::GetCANStatusReadable(uint32_t canBusChannelID) // TODO
{
	std::cerr << "CANDriverSocketCAN::GetCANStatusReadable not implemented" << std::endl;
	std::map<std::string, bool> status;
	return status;
}


void CANDriverSocketCAN::receiveLoop() // TODO: read errors, call onErrorCallback(&errorMsg);
{
	while(!done)
	{
		// wait for data getting received
		struct pollfd fd = {.fd = canSocket, .events = POLLIN};
		int pollRet = poll(&fd, 1, 200); // 200ms timeout
		if(pollRet < 0)
		{
			std::cerr << "Error polling canSocket" << errno << std::endl;
			done = true;
			return;
		}
		if(pollRet == 0) // timeout
		{
//			std::cout << "canSocket poll timeout" << std::endl;
			if(done) break;
			continue;
		}

		struct canfd_frame frame;
		int size = sizeof(frame);
		int readLength = read(canSocket, &frame, size);
		if(readLength < 0) throw std::runtime_error("CAN read failed");
		else if(readLength < size) throw std::runtime_error("CAN read incomplete");

		//TODO: switch timestamp to current unix time
		struct timeval timestamp;
		ioctl(canSocket, SIOCGSTAMP, &timestamp);
		uint64_t timestamp_us = (uint64_t)timestamp.tv_sec * 1e6 + (uint64_t)timestamp.tv_usec;

		uint8_t canChannelID = 0; // TODO support multiple devices

		frame.can_id &= 0x1FFFFFFF;

		try
		{
			onRecvCallback(canChannelID, frame.can_id, frame.data, frame.len, timestamp_us, this);
		}
		catch(const std::exception& e)
		{
			Debug::error("CANDriverSocketCAN::receiveLoop error: %s", e.what());
		}
		
	}
}
