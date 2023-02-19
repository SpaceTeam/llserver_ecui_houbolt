#ifndef CONTROL_LOOP_WORKER
#define CONTROL_LOOP_WORKER

#include <thread>
#include <memory>
#include <string>

#include "utility/ring_buffer.hpp"
#include "peripherie/input_message.hpp"
#include "peripherie/output_message.hpp"
#include "web/input_message.hpp"
#include "web/output_message.hpp"

//#include "state.h"
#include "config.hpp"

class control_loop_worker
{
private:
	using peripherie_output_t = std::shared_ptr<ring_buffer<peripherie::output_message, output_message_buffer_capacity>>;
	using peripherie_input_t = std::shared_ptr<ring_buffer<peripherie::input_message, input_message_buffer_capacity>>;

	using web_output_t = std::shared_ptr<ring_buffer<web::output_message, web_message_buffer_capacity>>;
	using web_input_t = std::shared_ptr<ring_buffer<web::input_message, web_message_buffer_capacity>>;

	std::jthread control_thread;

public:
	struct message_buffers
	{
		peripherie_input_t  peripherie_input;
		peripherie_output_t peripherie_output;
		web_input_t         web_input;
		web_output_t        web_output;
	};

	// constructor
	control_loop_worker(void) = delete;
	explicit control_loop_worker(struct message_buffers message_buffers);

	// destructor
	~control_loop_worker(void) = default;

	// non copyable
	control_loop_worker(control_loop_worker const &other) = delete;
	auto operator=(control_loop_worker const &other) -> control_loop_worker& = delete;

	// movable
	control_loop_worker(control_loop_worker &&other) noexcept = default;
	auto operator=(control_loop_worker &&other) noexcept -> control_loop_worker& = default;

	auto join(void) -> void;
	auto detach(void) -> void;
	auto request_stop(void) noexcept -> bool;

private:
	static auto control_loop(std::stop_token stop_token, struct message_buffers message_buffers) -> void;
};

#endif /* CONTROL_LOOP_WORKER */
