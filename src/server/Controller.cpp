#include "server/Controller.h"

#include "control_flag.h"

#include <iostream>
#include <thread>
#include <string>

Controller::Controller(
	std::shared_ptr<RingBuffer<std::string>>& request_queue,
	std::shared_ptr<RingBuffer<std::string>>& response_queue
) :
	socket("127.0.0.1", "8080"),
	request_queue(request_queue),
	response_queue(response_queue)
{
	return;
}


void
Controller::run(
	void
) {
	std::jthread read_thread(&Controller::read_loop, *this);
	write_loop();

	return;
}


void
Controller::read_loop(
	void
) {
	extern volatile sig_atomic_t finished;

	while(!finished) {
		request_queue->push(socket.receive());
	}

	return;
}


void
Controller::write_loop(
	void
) {
	extern volatile sig_atomic_t finished;

	while(!finished) {
		socket.send(request_queue->pop());
	}

	return;
}

