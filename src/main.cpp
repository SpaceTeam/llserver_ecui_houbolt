#include <atomic>
#include <cerrno>
#include <csignal>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <system_error>

#include <sys/mman.h>
#include <unistd.h>

#include "config.hpp"
#include "peripherie_worker.hpp"
#include "control_loop_worker.hpp"
#include "utility/ring_buffer.hpp"

#include "utility/logger.hpp"

struct options
{
	std::string config_path;
};

static std::atomic<bool> finished = false;

auto
usage(void) -> void
{
	extern char *__progname;

	std::cerr << "usage: " << __progname << " [-c configfile]" << std::endl;

	exit(EXIT_FAILURE);
}

auto get_options(int argument_count, char **arguments) -> options 
{
	extern char *optarg;

	struct options options{.config_path="config"};

	int option;
	while ((option = getopt(argument_count, arguments, "c:")) != -1)
	{
		switch (option)
		{
		case 'c':
			options.config_path = optarg;
			break;

		default:
			usage();
		};
	}

	return options;
}

auto
signal_handler(int signal) -> void
{
	extern std::atomic<bool> finished;

	switch (signal)
	{
	case SIGINT:
	case SIGTERM:
	case SIGABRT:
		finished = true;
		break;

	default:
		break;
	}

	return;
}

auto
setup_signal_handling(void) -> void
{
	struct sigaction signal_action;

	sigemptyset(&signal_action.sa_mask);
	signal_action.sa_handler = signal_handler;
	signal_action.sa_flags = 0;

	if (sigaction(SIGINT, &signal_action, NULL) == -1)
	{
		throw std::system_error(errno, std::generic_category(), "could not set signal handler for SIGINT");
	}

	if (sigaction(SIGTERM, &signal_action, NULL) == -1)
	{
		throw std::system_error(errno, std::generic_category(), "could not set signal handler for SIGTERM");
	}

	if (sigaction(SIGABRT, &signal_action, NULL) == -1)
	{
		throw std::system_error(errno, std::generic_category(), "could not set signal handler for SIGABRT");
	}

	log<severity::info>("main", "set signal handlers");

	return;
}


// TODO: check documentation of power management behaviour
/* Latency trick
 * if the file /dev/cpu_dma_latency exists,
 * open it and write a zero into it. This will tell 
 * the power management system not to transition to 
 * a high cstate (in fact, the system acts like idle=poll)
 * When the fd to /dev/cpu_dma_latency is closed, the behavior
 * goes back to the system default.
 * 
 * Documentation/power/pm_qos_interface.txt
 */
auto
set_latency_target(void) -> std::ofstream
{
	std::ofstream latency_target_file("/dev/cpu_dma_latency", std::ofstream::out);
	if (latency_target_file.is_open())
	{
		int32_t latency_target_value = 0;

		latency_target_file << latency_target_value;
	}

	log<severity::info>("main", "turned off management");

	// NOTE(Lukas Karafiat): file handle should be left open as power management would be reset
	return latency_target_file;
}

auto
lock_memory(void) -> void
{
	int error;

	// NOTE(Lukas Karafiat): Lock memory to ensure no swapping is done. Needed for performance reasons.
	error = mlockall(MCL_CURRENT | MCL_FUTURE);
	if (error == -1)
	{
		throw std::system_error(errno, std::generic_category(), "could not lock memory");
	}

	log<severity::info>("main","locked all of the program's virtual address space into RAM\n");

	return;
}

auto
main(int32_t argument_count, char *arguments[]) -> int32_t
{
	auto options = get_options(argument_count, arguments);
	argument_count -= optind;
	arguments += optind;

	if (argument_count != 0)
	{
		usage();
	}

	log<severity::info>("main", "config path: " + options.config_path);

	// TODO: read config

	setup_signal_handling();

//	set_latency_target();
	lock_memory();

// 	______________        ________________        _______________
//	|            |        |              |        |             |
//	|            |N--2--N>|              |N--0--B>|             |
//	|            |        |              |        | peripherie  |
//	| web server |        | control loop |        |             |
//	|            |        |              |        | read/write  |
//	|            |        |              |        | sensor data |
//	|            |<N--3--N|              |<N--1--B|             |
//	|____________|        |______________|        |_____________|

	using peripherie_input_buffer  = ring_buffer<peripherie::input_message,  input_message_buffer_capacity>;
	using peripherie_output_buffer = ring_buffer<peripherie::output_message, output_message_buffer_capacity>;

	using web_input_buffer  = ring_buffer<web::input_message,  web_message_buffer_capacity>;
	using web_output_buffer = ring_buffer<web::output_message, web_message_buffer_capacity>;

	auto sensor_message_buffer   = std::make_shared<peripherie_input_buffer>();
	auto actuator_message_buffer = std::make_shared<peripherie_output_buffer>();

	auto web_input_message_buffer  = std::make_shared<web_input_buffer>();
	auto web_output_message_buffer = std::make_shared<web_output_buffer>();

	control_loop_worker::message_buffers control_loop_message_buffers =
	{
		.peripherie_input = sensor_message_buffer,
		.peripherie_output = actuator_message_buffer,
		.web_input = web_input_message_buffer,
		.web_output = web_output_message_buffer
	};

//	web_server w{web_input_message_buffer, web_output_message_buffer};
	peripherie_worker p{sensor_message_buffer, actuator_message_buffer};
	control_loop_worker c{control_loop_message_buffers};

	finished.wait(false);

	return EXIT_SUCCESS;
}

