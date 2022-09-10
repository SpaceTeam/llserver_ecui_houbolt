#include "Peripherie.h"

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
		sensor_queue->push(frame);

	} else if(error == -1) {
		throw std::system_error(errno, std::generic_category(), "could not receive peripherie frame from '" + std::string("vcan0") + "'");
	}

	return;
}

void
Peripherie::write_peripherie(
	void
) {
	std::optional<struct_peripherie_frame> optional_frame = actuator_queue->pop();

	if (!optional_frame) {
		return;
	}

	int error;

	struct peripherie_frame frame = optional_frame.value();
	switch (frame.protocol) {
	case CAN:
		error = can_socket.send_frame(frame);
		if (error != 0) {
			throw std::system_error(errno, std::generic_category(), "could not send peripherie frame to '" + std::string("vcan0") + "'");
		}
		break;

	default:
		log<WARNING>("peripherie write", "protocol not supported");
	}

	return;
}

