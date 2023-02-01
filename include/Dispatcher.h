#ifndef DISPATCHER_HPP
#define DISPATCHER_HPP

#include "utility/RingBuffer.h"
#include "utility/json.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <functional>
#include <any>
#include <variant>

#include "config.h"

enum command_names {
	set_states,
	start_periodic_transmission,
	stop_periodic_transmission,
	start_sequence,
	abort_sequence
};

struct set_states_info {
	std::vector<std::string> state_names;
	std::vector<double> values;
	std::vector<uint64_t> timestamps;
};


struct start_sequence_info{
	std::string commands;
	// TODO: implement
};

// std::variant
struct command{
	command_names command_name;
	std::variant<nullptr_t, set_states_info, start_sequence_info> additional_info;
};


class Dispatcher {
private:
	std::shared_ptr<RingBuffer<std::string, request_buffer_capacity, false, true>> request_queue;
	std::shared_ptr<RingBuffer<std::any, command_buffer_capacity, true, false>> command_queue;

	std::unordered_map<std::string, std::function<void(nlohmann::json)>> commands;

public:
	Dispatcher(void) = delete;
	// NOTE(Lukas Karafiat): the function declaration got out of hand, had to shorten it quite a bit
	explicit Dispatcher(
		std::shared_ptr<RingBuffer<std::string, request_buffer_capacity, false, true>>,
		std::shared_ptr<RingBuffer<std::any, command_buffer_capacity, true, false>>);
	~Dispatcher(void);

	// non copyable
	Dispatcher(const Dispatcher &) = delete;
	void operator=(const Dispatcher &) = delete;

	// movable
	Dispatcher(Dispatcher &&) = default;
	Dispatcher& operator=(Dispatcher &&) = default;

	void run(void);

private:
	static void set_states(nlohmann::json message);
	static void start_periodic_transmission(nlohmann::json message);
	static void stop_periodic_transmission(nlohmann::json message);
	static void start_sequence(nlohmann::json message);
	static void abort_sequence(nlohmann::json message);
};

#endif /* DISPATCHER_HPP */
