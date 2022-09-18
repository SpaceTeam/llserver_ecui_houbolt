#include "ControlLoop.h"

#include <linux/sched.h>

#include "control_flags.h"
#include "utility/Logger.h"

ControlLoop::ControlLoop(
	std::shared_ptr<RingBuffer<std::any>> &command_queue,
	std::shared_ptr<RingBuffer<std::string>> &response_queue,
	std::shared_ptr<RingBuffer<struct sensor>> &sensor_queue,
	std::shared_ptr<RingBuffer<struct actuator>> &actuator_queue
) :
	command_queue(command_queue),
	response_queue(response_queue),
	sensor_queue(sensor_queue),
	actuator_queue(actuator_queue)
{
	// read config and insert states

	// read config for sensor to state mapping
	// read config for state to actuator mapping

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
	extern sig_atomic_t volatile finished;

	// NOTE(Lukas Karafiat): how much time do we actually have? 20 ms = 20'000us = 20'000'000ns

	struct sched_param scheduling_parameters{.sched_priority = 80};
	sched_setscheduler(0, SCHED_RR, &scheduling_parameters);

	while (!finished) {
		std::pair<struct sensor[32], size_t> sensor_data{};

		// read all available sensor data
		sensor_queue->pop_all(sensor_data);

		// NOTE(LUKAS KARAFIAT): command data should not be sent every 20ms (this is not a video game!)
		// read sequence command or set state
		std::optional<std::any> command = command_queue->pop();
		// do stuff with command

		// (sensors, states, current_time) -> (states, actuators)

		// purely time based sequences -> states
		// purely input driven states -> states
		// timed state machine -> states
		// timed reactive state -> state

		std::pair<struct actuator[32], size_t> actuator_data{};

		// write all available actuator data
		actuator_queue->push_all(actuator_data);

		// NOTE(Lukas Karafiat): 20ms ~ 50fps, for a maximum of 1024 states which are 8Bytes large <3.2MBit/s of Bandwidth
		//     so we could technically send all states to the clients
		// response_queue.push_all(states);

		// wait for up to next 20ms

		sched_yield();
	}

	return;
}

