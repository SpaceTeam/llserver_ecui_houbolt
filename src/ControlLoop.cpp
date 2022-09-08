#include "ControlLoop.h"

ControlLoop::ControlLoop(
	std::shared_ptr<RingBuffer<std::any>>& command_queue,
	std::shared_ptr<RingBuffer<std::string>>& response_queue,
	std::shared_ptr<RingBuffer<struct peripherie_frame>>& sensor_queue,
	std::shared_ptr<RingBuffer<struct peripherie_frame>>& actuator_queue
) {
	return;
}


ControlLoop::~ControlLoop(
	void
) {
	return;
}


void
ControlLoop::run(
	void
) {
	return;
}

