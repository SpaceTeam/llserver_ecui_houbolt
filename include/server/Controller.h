#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include "Queue.h"

class Controller {
private:

public:
	Controller(Queue&, Queue&);
	void run(void);
};

#endif /* CONTROLLER_HPP */
