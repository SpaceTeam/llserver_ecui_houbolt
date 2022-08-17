#include "server/Queue.h"

void
Queue::push(
	int value
) {
	std::lock_guard<std::mutex> lock(write_mutex);

	buffer.push(value);
	unread_elements.release();	

	return;
}


int
Queue::pop(
	void
) {
	std::lock_guard<std::mutex> lock(read_mutex);

	unread_elements.acquire();
	int value = buffer.front();
	buffer.pop();

	return value;
}

