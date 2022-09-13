#include "Peripherie.h"

#include <string.h>

#include "control_flags.h"
#include "utility/Logger.h"

Peripherie::Peripherie(
	std::shared_ptr<RingBuffer<struct peripherie_frame>> &actuator_queue,
	std::shared_ptr<RingBuffer<struct peripherie_frame>> &sensor_queue
) :
	actuator_queue(actuator_queue),
	sensor_queue(sensor_queue),
	can_socket("vcan0")
{
	return;
}


Peripherie::~Peripherie(
	void
) {
	return;
}

void
Peripherie::run(
	void
) {
	extern volatile sig_atomic_t finished;

	// NOTE(Lukas Karafiat): use sched_set_scheduler to set scheduler for each thread as non RT-Tasks do not need RT scheduling
	// NOTE(Lukas Karafiat): lock program from swap

	while (!finished) {
		read_peripherie();
		write_peripherie();
	}

	return;
}


void
Peripherie::read_peripherie(
	void
) {
	int error;

	struct peripherie_frame frame;
	error = can_socket.receive_frame(frame);
	if (error == 0) {
		// TODO: make sure that push is guaranteed somehow
		sensor_queue->push(frame);

	} else if(error == -1) {
		log<ERROR>("peripherie-read", "could not receive peripherie frame from '" + std::string("vcan0") + "': " + strerror(errno));
	}

	return;
}


void
Peripherie::write_peripherie(
	void
) {
	auto frame = actuator_queue->pop();
	if (!frame.has_value()) {
		return;
	}

	switch (frame->protocol) {
	case CAN:
		can_socket.send_frame(*frame);
		break;

	default:
		log<WARNING>("peripherie write", "protocol not supported");
	}

	return;
}

