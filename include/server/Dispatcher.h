#ifndef DISPATCHER_HPP
#define DISPATCHER_HPP

#include "Queue.h"

class Dispatcher {
private:

public:
	Dispatcher(Queue&, Queue&);
	void run(void);
};

#endif /* DISPATCHER_HPP */
