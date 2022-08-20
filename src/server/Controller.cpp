#include "server/Controller.h"

#include <iostream>
#include <thread>

Controller::Controller(
	std::shared_ptr<RingBuffer<int>>& request_queue,
	std::shared_ptr<RingBuffer<int>>& response_queue
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
	extern bool finished;

	while(!finished) {
		socket.receive();
		request_queue->push(1);
	}

	return;
}


void
Controller::write_loop(
	void
) {
	extern bool finished;

	while(!finished) {
		//int value = request_queue->pop();
		std::string payload = "sample";
		socket.send(payload);
	}

	return;
}

