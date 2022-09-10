#ifndef PERIPHERIE_H
#define PERIPHERIE_H

#include "utility/RingBuffer.h"

#include "peripherie/Frame.h"
#include "peripherie/can/CANSocket.h"

#include <memory>
#include <string>

class Peripherie {
private:
	std::shared_ptr<RingBuffer<struct peripherie_frame>> actuator_queue;
	std::shared_ptr<RingBuffer<struct peripherie_frame>> sensor_queue;

	CANSocket can_socket;

public:
	Peripherie(void) = delete;
	// NOTE(Lukas Karafiat): the function declaration got out of hand, had to shorten it quite a bit
	explicit Peripherie(
		std::shared_ptr<RingBuffer<struct peripherie_frame>> &actuator_queue,
		std::shared_ptr<RingBuffer<struct peripherie_frame>> &sensor_queue);
	~Peripherie(void);

	// non copyable
	Peripherie(Peripherie const &) = delete;
	void operator=(Peripherie const &x) = delete;

	// movable
	Peripherie(Peripherie &&) = default;
	Peripherie& operator=(Peripherie &&x) = default;

	void run(void);

private:
	void read_peripherie(void);
	void write_peripherie(void);
};

#endif /* PERIPHERIE_H */
