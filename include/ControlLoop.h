#ifndef CONTROL_LOOP_H
#define CONTROL_LOOP_H

#include "utility/RingBuffer.h"

#include <memory>
#include <string>
#include <any>
#include <unordered_map>

#include "config.h"
#include "State.h"

class ControlLoop {
private:
	std::shared_ptr<RingBuffer<std::any>> command_queue;
	std::shared_ptr<RingBuffer<std::string>> response_queue;

	std::shared_ptr<RingBuffer<struct sensor>> sensor_queue;
	std::shared_ptr<RingBuffer<struct actuator>> actuator_queue;

	double states[maximum_node_id][maximum_channel_id][maximum_value_id];

public:
	ControlLoop(void) = delete;
	// NOTE(Lukas Karafiat): the function declaration got out of hand, had to shorten it quite a bit
	explicit ControlLoop(
		std::shared_ptr<RingBuffer<std::any>> &command_queue,
		std::shared_ptr<RingBuffer<std::string>> &response_queue,
		std::shared_ptr<RingBuffer<struct sensor>> &sensor_queue,
		std::shared_ptr<RingBuffer<struct actuator>> &actuator_queue);
	~ControlLoop(void);

	// non copyable
	ControlLoop(const ControlLoop &) = delete;
	void operator=(const ControlLoop &x) = delete;

	// movable
	ControlLoop(ControlLoop &&) = default;
	ControlLoop& operator=(ControlLoop &&x) = default;

	void run(void);
};

#endif /* CONTROL_LOOP_H */
