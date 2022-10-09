#ifndef CONTROL_LOOP_H
#define CONTROL_LOOP_H

#include "utility/RingBuffer.h"

#include <memory>
#include <string>
#include <any>
#include <unordered_map>

#include "config.h"
#include "state.h"

class ControlLoop {
private:
	std::shared_ptr<RingBuffer<std::any, command_buffer_capacity, true, false>> command_queue;
	std::shared_ptr<RingBuffer<std::string, response_buffer_capacity>> response_queue;

	std::shared_ptr<RingBuffer<sensor, sensor_buffer_capacity, true, false>> sensor_queue;
	std::shared_ptr<RingBuffer<actuator, actuator_buffer_capacity, false, true>> actuator_queue;

	double states[maximum_state_id];

public:
	ControlLoop(void) = delete;
	// NOTE(Lukas Karafiat): the function declaration got out of hand, had to shorten it quite a bit
	explicit ControlLoop(
		std::shared_ptr<RingBuffer<std::any, command_buffer_capacity, true, false>>,
		std::shared_ptr<RingBuffer<std::string, response_buffer_capacity>>,
		std::shared_ptr<RingBuffer<sensor, sensor_buffer_capacity, true, false>>,
		std::shared_ptr<RingBuffer<actuator, actuator_buffer_capacity, false, true>>);
	~ControlLoop(void);

	// non copyable
	ControlLoop(const ControlLoop &) = delete;
	void operator=(const ControlLoop &) = delete;

	// movable
	ControlLoop(ControlLoop &&) = default;
	ControlLoop& operator=(ControlLoop &&) = default;

	void run(void);
};

#endif /* CONTROL_LOOP_H */
