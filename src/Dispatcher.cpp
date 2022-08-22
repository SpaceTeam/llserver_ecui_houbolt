#include "Dispatcher.h"

#include "control_flag.h"

Dispatcher::Dispatcher(
	std::shared_ptr<RingBuffer<std::string>>& request_queue,
	std::shared_ptr<RingBuffer<std::string>>& response_queue
) :
	request_queue(request_queue),
	response_queue(response_queue)
{
	using namespace std;

	commands = {
		{ "states-load"         , nullptr                     },
		{ "states-get"          , get_states                  },
		{ "states-set"          , set_states                  },
		{ "states-start"        , start_periodic_transmission },
		{ "states-stop"         , stop_periodic_transmission  },
		{ "sequence-start"      , start_sequence              },
		{ "abort"               , abort_sequence              },
		{ "auto-abort-change"   , change_automatic_abort      },
		{ "send-postseq-comment", nullptr                     },
		{ "gui-mapping-load"    , nullptr                     },
		{ "commands-load"       , nullptr                     },
		{ "commands-set"        , nullptr                     },
		{ "pythonScript-start"  , nullptr                     },
	};

	return;
}


void
Dispatcher::run(
	void
) {
	std::string message;

	while (!finished) {
		try {
			message = request_queue->pop();

			commands.at(message)(message);

		} catch (const std::out_of_range& e) {
			throw std::runtime_error("command not supported: " + message);

		} catch (const std::bad_function_call& e) {
			throw std::runtime_error("not implemented: " + message);

		} catch (const std::exception& e) {
			// NOTE(Lukas Karafiat): commands can fail, these will be logged and ignored
			// TODO: logging

		} catch(...) {
			// NOTE(Lukas Karafiat): due to previous deadlock behaviour of push() and receive_message()
			//     a timeout will be thrown and finished flag has to be checked again
		}
	}

	return;
}

void
Dispatcher::get_states(
	std::string message
) {
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
	return;
}

void
Dispatcher::stop_periodic_transmission(
	std::string message
) {
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

void
Dispatcher::change_automatic_abort(
	std::string message
) {
	return;
}

