#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include "Socket.h"
#include "utility/RingBuffer.h"

#include <memory>

class Controller {
private:
	Socket socket;

	std::shared_ptr<RingBuffer<int>> request_queue;
	std::shared_ptr<RingBuffer<int>> response_queue;

	void read_loop(void);
	void write_loop(void);

public:
	Controller(std::shared_ptr<RingBuffer<int>>&, std::shared_ptr<RingBuffer<int>>&);
	~Controller() = default;

	void run(void);
};

#endif /* CONTROLLER_HPP */
