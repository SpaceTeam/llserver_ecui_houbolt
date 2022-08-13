//#include <string>
//#include <iomanip>	  // std::setprecision
//#include <sched.h>
//#include <csignal>
//#include <fstream>
//
//#include <sys/stat.h>
//
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <fcntl.h>
//
//#include "common.h"
//
//#include "LLController.h"
//#include "utility/Config.h"
//
//// SMELL: don't use macros for constants. (this can go wrong in bigger projects)
//#define CONFIG_PATH_FILE "configPath.txt"
//// IMPROVE: use bool for running = true, running = false. more readable
//sig_atomic_t running = 1;
//// SMELL: unused global variable (maybe compile with -Wall -pedantic)
//sig_atomic_t signum = 0;
//
////#define TEST_LLSERVER
//
////#ifdef TEST_LLSERVER
//#include <thread>
//#include "can/CANManager.h"
//#include "can_houbolt/can_cmds.h"
//#include "can_houbolt/channels/generic_channel_def.h"
//#include <utility>
//#include <string>
//#include "utility/utils.h"
//// SMELL: Either use a constant or no global variable at all! no macros (if you can afford not to use a macro it is the right decision)
//#define CAN_TEST_NODE_ID 12

//#define TEST_NODE_INIT
//#define TEST_SPEAKER
//#define TEST_DATA

/*
std::thread *testThread = nullptr;
// SMELL: extract into separate program
// IMPROVE: use descriptive test function names.
// SMELL: to complex test case split into multiple testcases
void testFnc() {
	try {
	//wait a sec before executing function
	using namespace std::chrono_literals;
	std::this_thread::sleep_for(2000ms);

	CANManager *manager = CANManager::Instance();

//#ifdef TEST_NODE_INIT

	Can_MessageData_t msg = {0};
	msg.bit.info.buffer = DIRECT_BUFFER;
	msg.bit.info.channel_id = GENERIC_CHANNEL_ID;
	msg.bit.cmd_id = GENERIC_RES_NODE_INFO;
	NodeInfoMsg_t *info = (NodeInfoMsg_t *) msg.bit.data.uint8;
	info->firmware_version = 0xafb32dac;
	info->channel_mask = 0x0000001F;
	info->channel_type[0] = CHANNEL_TYPE_ADC24;
	// info->channel_type[1] = CHANNEL_TYPE_ADC16;
	info->channel_type[1] = CHANNEL_TYPE_DIGITAL_OUT;
	info->channel_type[2] = CHANNEL_TYPE_DIGITAL_OUT;
	info->channel_type[3] = CHANNEL_TYPE_DIGITAL_OUT;
	info->channel_type[4] = CHANNEL_TYPE_PNEUMATIC_VALVE;
	Can_MessageId_t canID = {0};
	canID.info.direction = 0;
	canID.info.priority = STANDARD_PRIORITY;
	canID.info.special_cmd = STANDARD_SPECIAL_CMD;
	canID.info.node_id = CAN_TEST_NODE_ID;

	manager->OnCANInit(0, canID.uint32, msg.uint8, sizeof(msg), utils::getCurrentTimestamp());

	std::this_thread::sleep_for(1000ms);

//#endif

//#ifdef TEST_DATA

	msg = {0};
	msg.bit.info.buffer = DIRECT_BUFFER;
	msg.bit.info.channel_id = GENERIC_CHANNEL_ID;
	msg.bit.cmd_id = GENERIC_RES_DATA;
	SensorMsg_t sensorMsg = {0};
	sensorMsg.channel_mask = 0x0000001D;

	Can_MessageId_t dataCanID = {0};
	dataCanID.info.direction = 1;
	dataCanID.info.priority = STANDARD_PRIORITY;
	dataCanID.info.special_cmd = STANDARD_SPECIAL_CMD;
	dataCanID.info.node_id = CAN_TEST_NODE_ID;

	uint8_t counter = 0;
	while (running) {
		//adc24
		sensorMsg.channel_data[0] = 0x20;
		sensorMsg.channel_data[1] = 0x00;
		sensorMsg.channel_data[2] = counter;
		//digital out
		sensorMsg.channel_data[3] = counter;
		sensorMsg.channel_data[4] = 0x04;
		sensorMsg.channel_data[5] = counter;
		sensorMsg.channel_data[6] = 0x00;
		sensorMsg.channel_data[7] = 0x00;
		sensorMsg.channel_data[8] = 0x00;
		sensorMsg.channel_data[9] = counter;
		sensorMsg.channel_data[10] = 0xA0;
		std::copy_n((uint8_t *) &sensorMsg.channel_mask, 4, msg.bit.data.uint8);
		std::copy_n(sensorMsg.channel_data, 11, &msg.bit.data.uint8[4]);


		manager->OnCANRecv(0, dataCanID.uint32, msg.uint8, sizeof(msg), utils::getCurrentTimestamp());
		std::this_thread::sleep_for(100ms);
		counter++;
	}
//#endif

//#ifdef TEST_SPEAKER
	while (!controller->IsInitialized()) {
		std::this_thread::sleep_for(1000ms);
	}
	Debug::error("setting test node state...");
	StateController *stateController = StateController::Instance();
	std::vector<std::string> states = {"testNode", "testNodeGetDiv", "testNodeSetDiv"};
	stateController->AddUninitializedStates(states);
	stateController->SetState("testNode", 1, std::chrono::high_resolution_clock::now().time_since_epoch().count());
	stateController->SetState("testNodeGetDiv", 1,
				  std::chrono::high_resolution_clock::now().time_since_epoch().count());
	stateController->SetState("testNodeSetDiv", 1,
				  std::chrono::high_resolution_clock::now().time_since_epoch().count());
//#endif
	}
	catch (std::exception &e) {
	Debug::print("error: %s", e.what());
	testThread = nullptr;
	raise(SIGTERM);
	}
}

#endif

void signalHandler(int signum) {
	running = 0;
	// SMELL: absolute useless statement.
	signum = signum;
	// BUG: not signal safe. (join, Debug::error,close,exit ,...) not signal safe. This function is a nightmare for safety!!!
	// IMPROVE: exit outside of main. (exit should only be used in main)
#ifdef TEST_LLSERVER
	if (testThread != nullptr && testThread->joinable())
	{
	testThread->join();
	delete testThread;
	}

#endif

	Debug::error("posix signal fired: %d, shutting down...", signum);

	try {
	LLController::Destroy();
	}
	catch (std::exception &e) {
	Debug::error("signal handler: failed to shutdown LLController, %s", e.what());
	}

	Debug::close();

	exit(signum);
}
*/








#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include <csignal>
#include <cerrno>

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
	extern int errno;

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

	set_scheduling_priority(60);

	set_latency_target();

/*
	// SMELL: don't test the program with itself use a test program
#ifdef TEST_LLSERVER
	testThread = new std::thread(testFnc);
#endif
	// IMPROVE: use normal class with shared pointers: This is cleaner and more efficient.
	LLController *llController = LLController::Instance();
	llController->Init(options.config_path);
	// SMELL: not used string
	std::string inputStr;
	// SMELL: NOT BUSY WAITING !!! Why not abandon thread?
	while (running) {
		sleep(1);
	}
*/
}

