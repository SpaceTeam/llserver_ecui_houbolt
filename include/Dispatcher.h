#ifndef DISPATCHER_HPP
#define DISPATCHER_HPP

#include "utility/RingBuffer.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <functional>
#include <any>

// command = ( sequence | state )

class Dispatcher {
private:
	std::shared_ptr<RingBuffer<std::string>> request_queue;
	std::shared_ptr<RingBuffer<std::any>> command_queue;

	std::unordered_map<std::string, std::function<void(std::string)>> commands;

public:
	Dispatcher(void) = delete;
	// NOTE(Lukas Karafiat): the function declaration got out of hand, had to shorten it quite a bit
	explicit Dispatcher(
		std::shared_ptr<RingBuffer<std::string>>,
		std::shared_ptr<RingBuffer<std::any>>);
	~Dispatcher(void);

	// non copyable
	Dispatcher(const Dispatcher &) = delete;
	void operator=(const Dispatcher &) = delete;

	// movable
	Dispatcher(Dispatcher &&) = default;
	Dispatcher& operator=(Dispatcher &&) = default;

	void run(void);

private:
	static void set_states(std::string);
	static void start_periodic_transmission(std::string);
	static void stop_periodic_transmission(std::string);
	static void start_sequence(std::string);
	static void abort_sequence(std::string);
};

#endif /* DISPATCHER_HPP */
