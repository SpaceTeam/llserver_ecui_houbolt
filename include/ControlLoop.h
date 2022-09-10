#ifndef CONTROL_LOOP_H
#define CONTROL_LOOP_H

#include "utility/RingBuffer.h"

#include "peripherie/Frame.h"

#include <memory>
#include <string>
#include <any>

class ControlLoop {
private:
	std::shared_ptr<RingBuffer<std::string>> request_queue;
	std::shared_ptr<RingBuffer<std::string>> response_queue;

	std::shared_ptr<RingBuffer<std::string>> input_queue;
	std::shared_ptr<RingBuffer<std::string>> output_queue;

public:
	ControlLoop(void) = delete;
	// NOTE(Lukas Karafiat): the function declaration got out of hand, had to shorten it quite a bit
	explicit ControlLoop(
		std::shared_ptr<RingBuffer<std::any>>& command_queue,
		std::shared_ptr<RingBuffer<std::string>>& response_queue,
		std::shared_ptr<RingBuffer<struct peripherie_frame>>& sensor_queue,
		std::shared_ptr<RingBuffer<struct peripherie_frame>>& actuator_queue);
	~ControlLoop(void);

	// non copyable
	ControlLoop(ControlLoop const &) = delete;
	void operator=(ControlLoop const &x) = delete;

	// movable
	ControlLoop(ControlLoop &&) = default;
	ControlLoop& operator=(ControlLoop &&x) = default;

	void run(void);
};

#endif /* CONTROL_LOOP_H */
