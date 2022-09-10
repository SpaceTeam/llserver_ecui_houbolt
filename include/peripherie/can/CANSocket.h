#ifndef PERIPHERIE_CAN_SOCKET_HPP
#define PERIPHERIE_CAN_SOCKET_HPP

#include <string>

#include "peripherie/Frame.h"

class CANSocket {
private:
	int socket_fd;

public:
	CANSocket(void) = delete;
	explicit CANSocket(std::string interface_name);
	~CANSocket(void);

	// non copyable
	CANSocket(CANSocket const &) = delete;
	void operator=(CANSocket const &x) = delete;

	// movable
	CANSocket(CANSocket &&) = default;
	CANSocket& operator=(CANSocket &&x) = default;

	int receive_frame(struct peripherie_frame &peripherie_frame);
	int send_frame(struct peripherie_frame &peripherie_frame);
};

#endif /* PERIPHERIE_CAN_SOCKET_HPP */
