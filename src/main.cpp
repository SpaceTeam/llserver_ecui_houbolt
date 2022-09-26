#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include <csignal>
#include <cerrno>

#include <sys/mman.h>

#include <thread>
#include <system_error>
#include <memory>
#include <any>

#include "control_flags.h"

#include "WebSocket.h"
#include "Dispatcher.h"
#include "ControlLoop.h"
#include "Peripherie.h"
#include "utility/RingBuffer.h"
#include "utility/Logger.h"

struct options {
	std::string config_path;
};

sig_atomic_t volatile finished = false;
sig_atomic_t volatile log_peripherie_data = false;


void
usage(
	void
) {
	extern char *__progname;

	std::cerr << "usage: " << __progname << " [-c configfile]" << std::endl;

	exit(EXIT_FAILURE);
}


struct options
get_options(
	int argc,
	char **argv
) {
	extern char *optarg;

	struct options options{.config_path="config"};

	int option;
	while ((option = getopt(argc, argv, "c:")) != -1) {
		switch (option) {
		case 'c':
			options.config_path = optarg;
			break;

		default:
			usage();
		};
	}

	return options;
}


void
signal_handler(
	int signal
) {
	extern sig_atomic_t volatile finished;

	switch (signal) {
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


void
setup_signal_handling(
	void
) {
	struct sigaction signal_action{};

	signal_action.sa_handler = signal_handler;
	signal_action.sa_flags = 0;

	if (sigaction(SIGINT, &signal_action, NULL) == -1) {
		throw std::system_error(errno, std::generic_category(), "could not set signal handler for SIGINT");
	}

	if (sigaction(SIGTERM, &signal_action, NULL) == -1) {
		throw std::system_error(errno, std::generic_category(), "could not set signal handler for SIGTERM");
	}

	if (sigaction(SIGABRT, &signal_action, NULL) == -1) {
		throw std::system_error(errno, std::generic_category(), "could not set signal handler for SIGABRT");
	}

	log<Severity::INFO>("main", "set signal handlers");

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
std::ofstream
set_latency_target(
	void
) {
	std::ofstream latency_target_file("/dev/cpu_dma_latency", std::ofstream::out);
	if (latency_target_file.is_open()) {
		int32_t latency_target_value = 0;

		latency_target_file << latency_target_value;
	}

	log<Severity::INFO>("main", "turned off management");

	// NOTE(Lukas Karafiat): file handle should be left open as power management would be reset
	return latency_target_file;
}

void
lock_memory(
	void
) {
	int error;

	// NOTE(Lukas Karafiat): Lock memory to ensure no swapping is done.
	error = mlockall(MCL_CURRENT | MCL_FUTURE);
	if(error == -1) {
		throw std::system_error(errno, std::generic_category(), "could not lock memory");
	}

	log<INFO>("main","locked all of the program's virtual address space into RAM\n");

	return;
}

int
main(
	int argc,
	char *argv[]
) {
	auto options = get_options(argc, argv);
	argc -= optind;
	argv += optind;

	if (argc != 0) {
		usage();
	}

	log<Severity::INFO>("main", "config path: " + options.config_path);

	// TODO: read config

	setup_signal_handling();
//	set_latency_target();
	lock_memory();

// 	______________      _______________________      ________________      _______________
//	|            |      |                     |      |              |      |             |
//	|            |--2-->| command interpreter |##4##>|              |--0-->|             |
//	|            |      |    and dispatcher   |#####>|              |      | peripherie  |
//	| web server |      |_____________________|      | control loop |      |             |
//	|            |                                   |              |      | read/write  |
//	|            |                                   |              |      | sensor data |
//	|            |<----------------3-----------------|              |<--1--|             |
//	|____________|                                   |______________|      |_____________|

	auto   sensor_queue = std::make_shared<RingBuffer<struct sensor>>();
	auto actuator_queue = std::make_shared<RingBuffer<struct actuator>>();
	auto  request_queue = std::make_shared<RingBuffer<std::string>>();
	auto response_queue = std::make_shared<RingBuffer<std::string>>();
	auto  command_queue = std::make_shared<RingBuffer<std::any>>();

	auto   web_socket = WebSocket<1024>("8080", response_queue, request_queue);
	auto   dispatcher = Dispatcher(request_queue, command_queue);
	auto control_loop = ControlLoop(command_queue, response_queue, sensor_queue, actuator_queue);
	auto   peripherie = Peripherie(actuator_queue, sensor_queue);

	auto   peripherie_thread = std::jthread(&Peripherie::run, std::move(peripherie));
	auto control_loop_thread = std::jthread(&ControlLoop::run, std::move(control_loop));
	auto   dispatcher_thread = std::jthread(&Dispatcher::run, std::move(dispatcher));
	auto   web_socket_thread = std::jthread(&WebSocket<1024>::run, std::move(web_socket));

	return EXIT_SUCCESS;
}

