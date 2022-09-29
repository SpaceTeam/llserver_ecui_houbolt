#include "Peripherie.h"

#include <cstring>

#include "control_flags.h"
#include "utility/Logger.h"

Peripherie::Peripherie(
	std::shared_ptr<RingBuffer<actuator>> actuator_queue,
	std::shared_ptr<RingBuffer<sensor>> sensor_queue
) :
	actuator_queue(actuator_queue),
	sensor_queue(sensor_queue),
	can_socket_interface_name("vcan0"),
	can_socket(can_socket_interface_name)
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
	extern sig_atomic_t volatile finished;

	struct sched_param scheduling_parameters{.sched_priority = 60};
	sched_setscheduler(0, SCHED_RR, &scheduling_parameters);

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
	auto sensors_or_error = can_socket.receive_frame();

	if (std::holds_alternative<int>(sensors_or_error)) {
		log<ERROR>("peripherie.read_peripherie", "could not receive sensor data");
		return;
	}

	auto sensors = std::get<0>(sensors_or_error);

	if (0 < sensors.second) {
		bool delivered;

		delivered = sensor_queue->push_all(sensors);
		if (!delivered) {
			log<ERROR>("peripherie.read_peripherie", "could not send sensor data to control loop: sensor buffer full");
		}
	}

	return;
}


void
Peripherie::write_peripherie(
	void
) {
	auto actuator = actuator_queue->pop();
	if (!actuator.has_value()) {
		return;
	}

	std::optional<int> error;

	switch (actuator->peripherie_type) {
	case peripherie::type::can_socket:
		error = can_socket.send_frame(*actuator);
		break;

	default:
		log<WARNING>("peripherie.write_peripherie", "peripherie protocol not supported");
		break;
	}

	if (error.has_value()) {
		log<ERROR>("peripherie.write_peripherie", "could not send actuator data");
	}

	return;
}

