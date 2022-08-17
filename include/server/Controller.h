#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include "Socket.h"
#include "Queue.h"

#include <memory>

class Controller {
private:
	Socket socket;

	std::shared_ptr<Queue> request_queue;
	std::shared_ptr<Queue> response_queue;

	void read_loop(void);
	void write_loop(void);

public:
	Controller(std::shared_ptr<Queue>&, std::shared_ptr<Queue>&);
	~Controller() = default;

	void run(void);
};

#endif /* CONTROLLER_HPP */
