#ifndef DISPATCHER_HPP
#define DISPATCHER_HPP

#include "Queue.h"

#include <memory>

class Dispatcher {
private:

public:
	Dispatcher(std::shared_ptr<Queue>&, std::shared_ptr<Queue>&);

	void run(void);
};

#endif /* DISPATCHER_HPP */
