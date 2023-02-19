#ifndef PERIPHERIE_WORKER
#define PERIPHERIE_WORKER

#include <thread>
#include <memory>
#include <string>

#include "utility/ring_buffer.hpp"
#include "peripherie/input_message.hpp"
#include "peripherie/output_message.hpp"

//#include "state.h"
#include "config.hpp"

class peripherie_worker
{
private:
	using shared_output_message_buffer = std::shared_ptr<ring_buffer<peripherie::output_message, output_message_buffer_capacity>>;
	using shared_input_message_buffer = std::shared_ptr<ring_buffer<peripherie::input_message, input_message_buffer_capacity>>;

	std::jthread read_thread;
	std::jthread write_thread;

public:
	// constructor
	peripherie_worker(void) = delete;
	explicit peripherie_worker(shared_input_message_buffer input_message_buffer, shared_output_message_buffer output_message_buffer);

	// destructor
	~peripherie_worker(void) = default;

	// non copyable
	peripherie_worker(peripherie_worker const &other) = delete;
	auto operator=(peripherie_worker const &other) -> peripherie_worker& = delete;

	// movable
	peripherie_worker(peripherie_worker &&other) noexcept = default;
	auto operator=(peripherie_worker &&other) noexcept -> peripherie_worker& = default;

	auto join(void) -> void;
	auto detach(void) -> void;
	auto request_stop(void) noexcept -> bool;

private:
	static auto read_loop(std::stop_token stop_token, shared_input_message_buffer input_message_buffer) -> void;
	static auto write_loop(std::stop_token stop_token, shared_output_message_buffer output_message_buffer) -> void;
};

#endif /* PERIPHERIE_WORKER */
