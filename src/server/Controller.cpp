#include "server/Controller.h"

#include "control_flag.h"

#include <iostream>
#include <thread>
#include <string>
#include <optional>

Controller::Controller(
	std::shared_ptr<RingBuffer<std::string>>& request_queue,
	std::shared_ptr<RingBuffer<std::string>>& response_queue
) :
	socket("127.0.0.1", "8080"),
	request_queue(request_queue),
	response_queue(response_queue)
{
	return;
}


void
Controller::run(
	void
) {
	std::jthread read_thread(&Controller::read_loop, *this);
	write_loop();

	return;
}


void
Controller::read_loop(
	void
) {
	extern volatile sig_atomic_t finished;

	std::optional<std::string> tmp = std::nullopt;

	while(!finished) {
		try {
			if (tmp == std::nullopt) {
				tmp = std::make_optional<std::string>(socket.receive_message());
			}

			request_queue->push(tmp.value());
			tmp.reset();

		} catch (...) {
			// NOTE(Lukas Karafiat): due to previous deadlock behaviour of push() and receive_message()
			//     a timeout will be thrown and finished flag has to be checked again
		}
	}

	return;
}


void
Controller::write_loop(
	void
) {
	extern volatile sig_atomic_t finished;

	std::optional<std::string> tmp = std::nullopt;

	while(!finished) {
		try {
			if (tmp == std::nullopt) {
				tmp = std::make_optional<std::string>(request_queue->pop());
			}

			socket.send_message(tmp.value());
			tmp.reset();

		} catch (...) {
			// NOTE(Lukas Karafiat): due to previous deadlock behaviour of pop() and send_message()
			//     a timeout will be thrown and finished flag has to be checked again
		}
	}

	return;
}

