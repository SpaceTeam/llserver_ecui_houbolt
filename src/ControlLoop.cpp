#include "ControlLoop.h"

#include "control_flags.h"
#include "utility/Logger.h"

ControlLoop::ControlLoop(
	std::shared_ptr<RingBuffer<std::any>> &command_queue,
	std::shared_ptr<RingBuffer<std::string>> &response_queue,
	std::shared_ptr<RingBuffer<struct peripherie_frame>> &sensor_queue,
	std::shared_ptr<RingBuffer<struct peripherie_frame>> &actuator_queue
) :
	command_queue(command_queue),
	response_queue(response_queue),
	sensor_queue(sensor_queue),
	actuator_queue(actuator_queue)
{
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
	extern volatile sig_atomic_t finished;

	while (!finished) {
		auto tmp = sensor_queue->pop();

		if (tmp) {
			log<DEBUG>("control loop", "sensor value from " + std::to_string(tmp.value().id));
		}
	}

	return;
}

