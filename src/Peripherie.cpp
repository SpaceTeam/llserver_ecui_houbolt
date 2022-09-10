#include "Peripherie.h"

#include "control_flags.h"

Peripherie::Peripherie(
	std::shared_ptr<RingBuffer<struct peripherie_frame>>&input_queue,
	std::shared_ptr<RingBuffer<struct peripherie_frame>>&output_queue
) {
	// TODO(Lukas Karafiat): setup CAN socket

	return;
}


Peripherie::~Peripherie(
	void
) {
	// close CAN socket

	return;
}

void
Peripherie::run(
	void
) {
	extern volatile sig_atomic_t finished;

	while (!finished) {

		// read CAN socket
		// write CAN socket
	}

	return;
}

