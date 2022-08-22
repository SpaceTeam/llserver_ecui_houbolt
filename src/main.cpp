#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include <csignal>
#include <cerrno>

#include <thread>
#include <system_error>
#include <memory>

#include "control_flag.h"

#include "WebSocket.h"
#include "Dispatcher.h"
#include "utility/RingBuffer.h"

struct options {
	std::string config_path = "config";
};

volatile sig_atomic_t finished = false;


void
usage(
	void
) {
	extern char *__progname;

	std::cerr << "usage: " << __progname << " [-c configfile]" << std::endl;

	exit(EXIT_FAILURE);
}


void
get_options(
	int argc,
	char **argv,
	struct options *options
) {
	extern char *optarg;

	int option;
	while ((option = getopt(argc, argv, "c:")) != -1) {
		switch (option) {
		case 'c':
			options->config_path = optarg;
			break;

		default:
			usage();
		};
	}

	return;
}


void
signal_handler(
	int signal
) {
	extern volatile sig_atomic_t finished;

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
	//extern int errno;

	if (signal(SIGINT, signal_handler) == SIG_ERR) {
		throw std::system_error(errno, std::generic_category(), "could not set signal handler for SIGINT");
	}

	if (signal(SIGTERM, signal_handler) == SIG_ERR) {
		throw std::system_error(errno, std::generic_category(), "could not set signal handler for SIGTERM");
	}

	if (signal(SIGABRT, signal_handler) == SIG_ERR) {
		throw std::system_error(errno, std::generic_category(), "could not set signal handler for SIGABRT");
	}

	return;
}


// IMPROVE: This makes the Code only compilable on linux. Make it optional for test version. (enables Multiplatform development)
void
set_scheduling_priority(
	int priority
) {
	struct sched_param scheduling_parameters{};
	scheduling_parameters.sched_priority = 60;

	sched_setscheduler(0, SCHED_FIFO, &scheduling_parameters);

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

	// NOTE(Lukas Karafiat): file handle should be left open as power management would be reset
	return latency_target_file;
}


int
main(
	int argc,
	char *argv[]
) {
	struct options options{};

	get_options(argc, argv, &options);
	argc -= optind;
	argv += optind;

	if (argc != 0) {
		usage();
	}

	setup_signal_handling();

	// TODO: read config

//	set_scheduling_priority(60);
//	set_latency_target();

	std::shared_ptr<RingBuffer<std::string>> response_queue(new RingBuffer<std::string>());
	std::shared_ptr<RingBuffer<std::string>> request_queue(new RingBuffer<std::string>());

	// NOTE(Lukas Karafiat): In a normal web server the dispatcher would
	//     be integrated into the controller, but if we want to hold more
	//     connections than one, a sequential dispatch of intructions has
	//     to be done, so I split it up.  Ordering will be done via a
	//     ring buffer.
	Dispatcher dispatcher = Dispatcher(response_queue, request_queue);
	WebSocket controller = WebSocket("8080", response_queue, request_queue);

	std::jthread dispatcher_thread(&Dispatcher::run, dispatcher);
	controller.run();

	return EXIT_SUCCESS;
}

