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

#define TEST_LLSERVER

#ifdef TEST_LLSERVER
#include <thread>
#include "can/CANManager.h"
#include "can_houbolt/can_cmds.h"
#include "can_houbolt/channels/generic_channel_def.h"

std::thread *testThread = nullptr;

typedef struct __attribute__((__packed__))
{
	uint32_t channel_mask;
	uint8_t *channel_data;
} SensorMsg_t;

void testFnc()
{
    //wait a sec before executing function
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1000ms);

    Can_MessageData_t msg;
    msg.bit.info.buffer = DIRECT_BUFFER;
    msg.bit.info.channel_id = GENERIC_CHANNEL_ID;
    msg.bit.cmd_id = GENERIC_RES_NODE_INFO;
    NodeInfoMsg_t info = {0};
    info.firmware_version = 10000;
    info.channel_mask = 0x0000000B;
    info.channel_type[0] = CHANNEL_TYPE_ADC24;
    info.channel_type[1] = CHANNEL_TYPE_ADC16;
    info.channel_type[2] = CHANNEL_TYPE_DIGITAL_OUT;
    info.channel_type[3] = CHANNEL_TYPE_SERVO;
    msg.bit.data.uint8 = (uint8_t *)&info;
    uint16_t canID = 0b00000000010;

    CANManager *manager = CANManager::Instance();
    manager->OnCANInit(0, canID, msg.uint8, sizeof(msg), 0x1);

    std::this_thread::sleep_for(1000ms);

    msg = {0};
    msg.bit.info.buffer = DIRECT_BUFFER;
    msg.bit.info.channel_id = GENERIC_CHANNEL_ID;
    msg.bit.cmd_id = GENERIC_RES_NODE_INFO;
    SensorMsg_t sensorMsg = {0};
    sensorMsg.channel_mask = 0x00000003;
    sensorMsg.channel_data = new uint8_t[3+2];
    while(running)
    {
        //adc24
        sensorMsg.channel_data[0] = 0x20;
        sensorMsg.channel_data[1] = 0x00;
        sensorMsg.channel_data[2] = 0x00;
        //adc 16
        sensorMsg.channel_data[3] = 0x00;
        sensorMsg.channel_data[4] = 0x04;

         manager->OnCANRecv(0, canID, msg.uint8, sizeof(msg), 0x1);
        std::this_thread::sleep_for(1000ms);
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

    ServerMode serverMode = ServerMode::LARGE_TESTSTAND;
    if (argc > 1)
    {
        if (strcmp(argv[1],"--smallTeststand") == 0)
        {
            serverMode = ServerMode::SMALL_TESTSTAND;
            printf("Using Small Teststand Profile...\n");
        }
        else if (strcmp(argv[1],"--smallOxfill") == 0)
        {
            serverMode = ServerMode::SMALL_OXFILL;
            printf("Using Small Oxfill Profile...\n");
        }
        else
        {
            printf("Defaulting to Franz Profile...\n");
        }
    }

#ifdef TEST_LLSERVER
    testThread = new std::thread(testFnc);

#endif

    LLController *llController = LLController::Instance();
    llController->Init(serverMode);

    std::string inputStr;
    while (running)
    {
	    sleep(1);
    }

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


    exit(signum);
}

