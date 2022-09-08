#include "Dispatcher.h"

#include "control_flags.h"

Dispatcher::Dispatcher(
	std::shared_ptr<RingBuffer<std::string>>& request_queue,
	std::shared_ptr<RingBuffer<std::any>>& command_queue
) :
	request_queue(request_queue),
	command_queue(command_queue)
{
	commands = {
		{ "states-load"         , nullptr                     },
		{ "states-get"          , nullptr                     },
		{ "states-set"          , set_states                  },
		{ "states-start"        , start_periodic_transmission },
		{ "states-stop"         , stop_periodic_transmission  },
		{ "sequence-start"      , start_sequence              },
		{ "abort"               , abort_sequence              },
		{ "auto-abort-change"   , nullptr                     },
		{ "send-postseq-comment", nullptr                     },
		{ "gui-mapping-load"    , nullptr                     },
		{ "commands-load"       , nullptr                     },
		{ "commands-set"        , nullptr                     },
		{ "pythonScript-start"  , nullptr                     },
	};

	return;
}


Dispatcher::~Dispatcher(
	void
) {
	return;
}


void
Dispatcher::run(
	void
) {
	std::string message;

	while (!finished) {
		std::optional<std::string> message_buffer = request_queue->pop();

		if (!message_buffer.has_value()) {
			continue;
		}

		message = message_buffer.value();

		try {
			commands.at(message)(message);

		} catch (const std::out_of_range& e) {
			throw std::runtime_error("command not supported: " + message);

		} catch (const std::bad_function_call& e) {
			throw std::runtime_error("not implemented: " + message);

		} catch (const std::exception& e) {
			// NOTE(Lukas Karafiat): commands can fail, these will be logged and ignored
			// TODO: logging
		}
	}

	return;
}

void
Dispatcher::set_states(
	std::string message
) {
	return;
}

void
Dispatcher::start_periodic_transmission(
	std::string message
) {
	log_peripherie_data = true;

	return;
}

void
Dispatcher::stop_periodic_transmission(
	std::string message
) {
	log_peripherie_data = false;

	return;
}

void
Dispatcher::start_sequence(
	std::string message
) {
	return;
}

void
Dispatcher::abort_sequence(
	std::string message
) {
	return;
}

