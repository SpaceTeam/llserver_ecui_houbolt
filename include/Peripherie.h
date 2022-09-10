#ifndef PERIPHERIE_H
#define PERIPHERIE_H

#include "utility/RingBuffer.h"

#include "peripherie/Frame.h"
#include "peripherie/can/CANSocket.h"

#include <memory>
#include <string>

class Peripherie {
private:
	std::shared_ptr<RingBuffer<struct peripherie_frame>> input_queue;
	std::shared_ptr<RingBuffer<struct peripherie_frame>> output_queue;

public:
	Peripherie(void) = delete;
	// NOTE(Lukas Karafiat): the function declaration got out of hand, had to shorten it quite a bit
	explicit Peripherie(
		std::shared_ptr<RingBuffer<struct peripherie_frame>>&input_queue,
		std::shared_ptr<RingBuffer<struct peripherie_frame>>&output_queue);
	~Peripherie(void);

	// non copyable
	Peripherie(Peripherie const &) = delete;
	void operator=(Peripherie const &x) = delete;

	// movable
	Peripherie(Peripherie &&) = default;
	Peripherie& operator=(Peripherie &&x) = default;

	void run(void);
};

#endif /* PERIPHERIE_H */
