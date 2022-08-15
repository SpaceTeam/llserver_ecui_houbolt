#include "server/Queue.h"

Queue::Queue(
	void
) {
	return;
}


Queue::~Queue(
	void
) {
	return;
}


void
Queue::push(
	int value
) {
	// 1. acquire lock(1)
	// 2. push
	// 3. increase semaphore
	// 4. release lock(1)

	return;
}


int
pop(
	void
) {
	// 1. acquire lock(2)
	// 2. pop
	// 3. decrease semaphore
	// 4. relesase lock(2)

	return 0;
}

