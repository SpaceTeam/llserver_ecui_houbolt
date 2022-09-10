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
		// read all available peripherie data
		auto tmp = sensor_queue->pop();
		if (tmp) {
			log<DEBUG>("control loop", "sensor value from " + std::to_string(tmp.value().id));
		}

		// transform peripherie data to sensor data
		// read sequence command or set state
		// (sensors, states) -> (states)
		// (sensors, states) -> (actuators)
		// transform actuator data to peripherie data
		// write all available actuator data
		// occasionally write state data to response_queue
	}

	return;
}

