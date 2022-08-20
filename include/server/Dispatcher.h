#ifndef DISPATCHER_HPP
#define DISPATCHER_HPP

#include "utility/RingBuffer.h"

#include <memory>

class Dispatcher {
private:

public:
	Dispatcher(std::shared_ptr<RingBuffer<int>>&, std::shared_ptr<RingBuffer<int>>&);

	void run(void);
};

#endif /* DISPATCHER_HPP */
