#include "control_loop_worker.hpp"

#include <chrono>

#include "utility/logger.hpp"

// constructor
control_loop_worker::control_loop_worker(struct message_buffers message_buffers):
	control_thread(control_loop, message_buffers)
{}

auto
control_loop_worker::join(void) -> void
{
	control_thread.join();
}

auto
control_loop_worker::detach(void) -> void
{
	control_thread.detach();
}

auto
control_loop_worker::request_stop(void) noexcept -> bool
{
	return control_thread.request_stop();
}

auto
control_loop_worker::control_loop(
	std::stop_token stop_token,
	struct message_buffers message_buffers
) -> void
{
	using namespace std::literals;

//	pthread_setname_np(pthread_self(), (connection.name() + " writer").c_str());

	sched_param scheduling_parameters{.sched_priority = 60};
	sched_setscheduler(0, SCHED_FIFO, &scheduling_parameters);

	log<severity::info>("control_loop_worker.control_loop", "set scheduler to round robin");

	std::vector<peripherie::input_message> peripherie_input_messages;
	std::vector<web::input_message> web_input_messages;

	std::vector<peripherie::output_message> peripherie_output_messages;
	std::vector<web::output_message> web_output_messages;

	peripherie_input_messages.reserve(input_message_buffer_capacity);
	web_input_messages.reserve(web_message_buffer_capacity);

	peripherie_output_messages.reserve(output_message_buffer_capacity);
	web_output_messages.reserve(web_message_buffer_capacity);

	while (!stop_token.stop_requested())
	{
		peripherie_input_messages.clear();
		peripherie_input_messages = message_buffers.peripherie_input->pop_all(std::move(peripherie_input_messages));

		web_input_messages.clear();
		web_input_messages = message_buffers.web_input->pop_all(std::move(web_input_messages));

		// process messages and update states

		// call plugin functions (state) -> (state)

		bool delivered;

		delivered = message_buffers.peripherie_output->try_push_all(peripherie_output_messages);
		if (!delivered)
		{
			log<severity::error>("control_loop_worker.control_loop", "could not send messages to peripherie buffer full");
		}

		delivered = message_buffers.web_output->try_push_all(web_output_messages);
		if (!delivered)
		{
			log<severity::error>("control_loop_worker.control_loop", "could not send messages to web buffer full");
		}
	}
}

