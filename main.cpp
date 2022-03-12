#include <string>
#include <iomanip>      // std::setprecision
#include <sched.h>
#include <csignal>

#include <sys/stat.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "common.h"

#include "LLController.h"
#include "utility/Config.h"

sig_atomic_t running = 1;
sig_atomic_t signum = 0;

//#define TEST_LLSERVER

#ifdef TEST_LLSERVER
#include <thread>
#include "can/CANManager.h"
#include "can_houbolt/can_cmds.h"
#include "can_houbolt/channels/generic_channel_def.h"
#include <utility>
#include <string>
#include "utility/utils.h"

#define CAN_TEST_NODE_ID 12

#define TEST_NODE_INIT
//#define TEST_SPEAKER
#define TEST_DATA

std::thread *testThread = nullptr;

void testFnc()
{
    try{

    
    //wait a sec before executing function
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(2000ms);

    CANManager *manager = CANManager::Instance();
    
#ifdef TEST_NODE_INIT

    Can_MessageData_t msg = {0};
    msg.bit.info.buffer = DIRECT_BUFFER;
    msg.bit.info.channel_id = GENERIC_CHANNEL_ID;
    msg.bit.cmd_id = GENERIC_RES_NODE_INFO;
    NodeInfoMsg_t *info = (NodeInfoMsg_t *)msg.bit.data.uint8;
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

#endif

#ifdef TEST_DATA

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
    while(running)
    {
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
        std::copy_n((uint8_t*)&sensorMsg.channel_mask, 4, msg.bit.data.uint8);
        std::copy_n(sensorMsg.channel_data, 11, &msg.bit.data.uint8[4]);


        manager->OnCANRecv(0, dataCanID.uint32, msg.uint8, sizeof(msg), utils::getCurrentTimestamp());
        std::this_thread::sleep_for(100ms);
        counter++;
    }
#endif

#ifdef TEST_SPEAKER
    while (!controller->IsInitialized())
    {
        std::this_thread::sleep_for(1000ms);
    }
    Debug::error("setting test node state...");
    StateController *stateController = StateController::Instance();
    std::vector<std::string> states = {"testNode", "testNodeGetDiv", "testNodeSetDiv"};
    stateController->AddUninitializedStates(states);
    stateController->SetState("testNode", 1, std::chrono::high_resolution_clock::now().time_since_epoch().count());
    stateController->SetState("testNodeGetDiv", 1, std::chrono::high_resolution_clock::now().time_since_epoch().count());
    stateController->SetState("testNodeSetDiv", 1, std::chrono::high_resolution_clock::now().time_since_epoch().count());
#endif
    }
    catch (std::exception &e)
    {
        Debug::print("error: %s", e.what());
        testThread = nullptr;
        raise(SIGTERM);
    }
}

#endif

static int latency_target_fd = -1;
static int32_t latency_target_value = 0;

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
static void set_latency_target(void)
{
    struct stat s;
    int ret;

    if (stat("/dev/cpu_dma_latency", &s) == 0) {
        latency_target_fd = open("/dev/cpu_dma_latency", O_RDWR);
        if (latency_target_fd == -1)
            return;
        ret = write(latency_target_fd, &latency_target_value, 4);
        if (ret == 0) {
            printf("# error setting cpu_dma_latency to %d!: %s\n", latency_target_value, strerror(errno));
            close(latency_target_fd);
            return;
        }
        printf("# /dev/cpu_dma_latency set to %dus\n", latency_target_value);
    }
}

void signalHandler(int signum)
{
    running = 0;
    signum = signum;

    #ifdef TEST_LLSERVER
        if (testThread != nullptr && testThread->joinable())
        {
            testThread->join();
            delete testThread;
        }

    #endif

    Debug::error("posix signal fired: %d, shutting down...", signum);

    try
    {
        LLController::Destroy();
    }
    catch (std::exception &e)
    {
        Debug::error("signal handler: failed to shutdown LLController, %s", e.what());
    }

    Debug::close();

    exit(signum);
}

int main(int argc, char const *argv[])
{
    system("clear");
    
	struct sched_param sp;
	sp.sched_priority = 60;

	sched_setscheduler(0, SCHED_FIFO, &sp);

    set_latency_target();

    // register signal SIGINT and signal handler
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGABRT, signalHandler);

    std::string configPath = "";
    if (argc > 1)
	{
    	std::string cfgPath(argv[1]);
    	configPath = cfgPath;
	}
    else
    {
        std::cerr << "No config file has been provided! Exiting..." << std::endl;
		exit(1);
    }

    #ifdef TEST_LLSERVER
    testThread = new std::thread(testFnc);
    #endif

    LLController *llController = LLController::Instance();
    llController->Init(configPath);


    std::string inputStr;
    while (1)
    {
	    sleep(1);
    }
}

