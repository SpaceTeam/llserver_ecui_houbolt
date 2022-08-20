#include "server/Dispatcher.h"

#include <iostream>

Dispatcher::Dispatcher(
	std::shared_ptr<RingBuffer<int>>& request_queue,
	std::shared_ptr<RingBuffer<int>>& response_queue
) {
	return;
}


void
Dispatcher::run(
	void
) {
	std::cout << "dispatcher" << std::endl;

	return;
}

/*	
// TODO: initializing

	while(running) {
		// wait for request

		// read request data and name

		std::string request_type;

		// TODO: use pattern matching when it is available in future versions of c++
		if (request_type == "states-get") {
			// TODO: get state

		} else if (request_type == "states-set") {
			// TODO: set state

		} else if (request_type == "states-start") {
			// TODO: start periodic state transmission

		} else if (request_type == "states-stop") {
			// TODO: stop periodic state transmission

		} else if (request_type == "sequence-start") {
			// TODO: start instruction sequence

		} else if (request_type == "abort") {
			// TODO: abort instruction sequence

		} else if (request_type == "auto-abort-change") {
			// TODO: set auto abort state
		}
	}

	// TODO: deinitializing
*/

