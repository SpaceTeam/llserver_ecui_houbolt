#ifndef PERIPHERIE_H
#define PERIPHERIE_H

#include <memory>
#include <string>

#include "utility/RingBuffer.h"
#include "peripherie/can/socket.h"

#include "state.h"

class Peripherie {
private:
	std::shared_ptr<RingBuffer<actuator>> actuator_queue;
	std::shared_ptr<RingBuffer<sensor>> sensor_queue;

	std::string can_socket_interface_name;
	peripherie::can::socket can_socket;

public:
	Peripherie(void) = delete;
	// NOTE(Lukas Karafiat): the function declaration got out of hand, had to shorten it quite a bit
	explicit Peripherie(
		std::shared_ptr<RingBuffer<actuator>> &actuator_queue,
		std::shared_ptr<RingBuffer<sensor>> &sensor_queue);
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
