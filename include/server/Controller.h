#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include "SocketClient.h"
#include "utility/RingBuffer.h"

#include <memory>

class Controller {
private:
	SocketClient socket;

	std::shared_ptr<RingBuffer<std::string>> request_queue;
	std::shared_ptr<RingBuffer<std::string>> response_queue;

	void read_loop(void);
	void write_loop(void);

public:
	Controller(std::shared_ptr<RingBuffer<std::string>>&request_queue, std::shared_ptr<RingBuffer<std::string>>&response_queue);
	~Controller() = default;

	void run(void);
};

#endif /* CONTROLLER_HPP */
