#ifndef DISPATCHER_HPP
#define DISPATCHER_HPP

#include "utility/RingBuffer.h"

#include <memory>
#include <string>

class Dispatcher {
private:

public:
	Dispatcher(std::shared_ptr<RingBuffer<std::string>>&, std::shared_ptr<RingBuffer<std::string>>&);

	void run(void);
};

#endif /* DISPATCHER_HPP */
