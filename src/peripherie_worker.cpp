#include "peripherie_worker.hpp"

#include <chrono>

#include "utility/logger.hpp"

// constructor
peripherie_worker::peripherie_worker(shared_input_message_buffer input_message_buffer, shared_output_message_buffer output_message_buffer):
	read_thread(read_loop, input_message_buffer),
	write_thread(write_loop, output_message_buffer)
{}

auto
peripherie_worker::join(void) -> void
{
	read_thread.join();
	write_thread.join();
}

auto
peripherie_worker::detach(void) -> void
{
	read_thread.detach();
	write_thread.detach();
}

auto
peripherie_worker::request_stop(void) noexcept -> bool
{
	bool made_stop_request = true;
	made_stop_request &= read_thread.request_stop();
	made_stop_request &= write_thread.request_stop();

	return made_stop_request;
}

auto
peripherie_worker::read_loop(std::stop_token stop_token, shared_input_message_buffer input_message_buffer) -> void
{
	using namespace std::literals;

//	pthread_setname_np(pthread_self(), (connection.name() + " writer").c_str());

	sched_param scheduling_parameters{.sched_priority = 60};
	sched_setscheduler(0, SCHED_RR, &scheduling_parameters);

//	log<severity::info>("peripherie_worker.read_loop of "s + connection.name(), "set scheduler to round robin");

	std::vector<peripherie::input_message> input_messages{};

	input_messages.reserve(input_message_buffer_capacity);

	while (!stop_token.stop_requested())
	{
		input_messages.clear();

		std::this_thread::sleep_for(1s);
//		input_messages = connection.try_read_frame_for(1s, std::move(input_messages));

		bool delivered = input_message_buffer->try_push_all(input_messages);
		if (!delivered)
		{
//			log<severity::error>("peripherie_worker.read_loop of "s + connection.name(), "could not send input data to control loop: input buffer full");
		}
	}
}

auto
peripherie_worker::write_loop(std::stop_token stop_token, shared_output_message_buffer output_message_buffer) -> void
{
	using namespace std::literals;

//	pthread_setname_np(pthread_self(), (connection.name() + " writer").c_str());

	sched_param scheduling_parameters{.sched_priority = 60};
	sched_setscheduler(0, SCHED_RR, &scheduling_parameters);

//	log<severity::info>("peripherie_worker.write_loop of "s + connection.name(), "set scheduler to round robin");

	while (!stop_token.stop_requested())
	{
		auto output_message = output_message_buffer->try_pop_for(1s);
		if (!output_message)
		{
			continue;
		}

		std::this_thread::sleep_for(1s);
/*
		bool delivered = connection.try_write_frame_for(1s, output_messages);
		if (!delivered)
		{
			log<severity::error>("peripherie_worker.write_loop of "s + connection.name(), "could not send output data");
		}
*/
	}
}

