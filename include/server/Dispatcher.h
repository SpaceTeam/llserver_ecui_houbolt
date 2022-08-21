#ifndef DISPATCHER_HPP
#define DISPATCHER_HPP

#include "utility/RingBuffer.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <functional>

class Dispatcher {
private:
	std::shared_ptr<RingBuffer<std::string>> request_queue;
	std::shared_ptr<RingBuffer<std::string>> response_queue;

	std::unordered_map<std::string, std::function<void(std::string)>> commands;

public:
	Dispatcher(std::shared_ptr<RingBuffer<std::string>>&, std::shared_ptr<RingBuffer<std::string>>&);

	void run(void);

private:
	static void get_states(std::string message);
	static void set_states(std::string message);
	static void start_periodic_transmission(std::string message);
	static void stop_periodic_transmission(std::string message);
	static void start_sequence(std::string message);
	static void abort_sequence(std::string message);
	static void change_automatic_abort(std::string message);
};

#endif /* DISPATCHER_HPP */
