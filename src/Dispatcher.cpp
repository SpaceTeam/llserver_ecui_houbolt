#include "Dispatcher.h"

#include "control_flags.h"
#include "utility/Logger.h"


Dispatcher::Dispatcher(
	std::shared_ptr<RingBuffer<std::string, request_buffer_capacity, false, true>> request_queue,
	std::shared_ptr<RingBuffer<std::any, command_buffer_capacity, true, false>> command_queue
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
	pthread_setname_np(pthread_self(), "dispatcher");

	std::string message;

	while (!finished) {
		std::optional<std::string> message_buffer = request_queue->pop();

		if (!message_buffer.has_value()) {
			continue;
		}

		message = message_buffer.value();
		log<DEBUG>("dispatcher", "got command '" + message + "'");

		nlohmann::json json_msg = nlohmann::json::parse(message);
		try {
			if(json_msg.find("type") != json_msg.end() ) {
				std::string type = json_msg["type"];
				commands.at(type)(json_msg);
			}
		} catch (const std::out_of_range &e) {
			// TODO(Lukas Karafiat): this should probably be a log message and no fatal exit
			throw std::runtime_error("command not supported: " + message);

		} catch (const std::bad_function_call &e) {
			// TODO(Lukas Karafiat): this should probably be a log message and no fatal exit
			throw std::runtime_error("not implemented: " + message);

		} catch (const std::exception &e) {
			// NOTE(Lukas Karafiat): commands can fail, these will be logged and ignored
			log<WARNING>("dispatcher", "command '" + message + "' failed");
		}
	}

	return;
}


void
Dispatcher::set_states(
	nlohmann::json message
) {
	// TODO(Christofer Held): figure out if additional information is needed
	std::vector<std::string> state_names;
	std::vector<double> values;
	std::vector<uint64_t> timestamps;
	for (auto state : message["content"]) {
		state_names.push_back(state["name"]);
		values.push_back(state["value"]);
		timestamps.push_back(state["timestamp"]);
	}

	return;
}


void
Dispatcher::start_periodic_transmission(
	nlohmann::json message
) {
	// TODO(Christofer Held): build command and add to queue
	log_peripherie_data = true;

	return;
}


void
Dispatcher::stop_periodic_transmission(
	nlohmann::json message
) {
	// TODO(Christofer Held): build command and add to queue
	log_peripherie_data = false;

	return;
}


void
Dispatcher::start_sequence(
	nlohmann::json message
) {
	// TODO: (Christofer Held) build command and add to queue
	nlohmann::json seq = message["content"][0];
	nlohmann::json abortSeq = message["content"][1];
	nlohmann::json comments = message["content"][2];
	//TODO(Christofer Held): check seq inputs from Sequence Handler

	return;
}


void
Dispatcher::abort_sequence(
	nlohmann::json message
) {
	// TODO: Send manual abort
	return;
}

