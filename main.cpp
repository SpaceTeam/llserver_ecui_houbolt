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

#define TEST_LLSERVER

#ifdef TEST_LLSERVER
#include <thread>
#include "can/CANManager.h"
#include "can_houbolt/can_cmds.h"
#include "can_houbolt/channels/generic_channel_def.h"

std::thread *testThread = nullptr;

typedef struct
{
    uint8_t msgType;
    NodeInfoMsg_t info;
} InfoMsgDummy_t;

void testFnc()
{
    //wait a sec before executing function
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1000ms);

    InfoMsgDummy_t msg;
    msg.msgType = GENERIC_RES_NODE_INFO;
    NodeInfoMsg_t *info = &msg.info;
    info->firmware_version = 10000;
    info->channel_mask = 0x000B;
    info->channel_type[0] = CHANNEL_TYPE_ADC24;
    info->channel_type[1] = CHANNEL_TYPE_ADC16;
    info->channel_type[2] = CHANNEL_TYPE_DIGITAL_OUT;
    info->channel_type[3] = CHANNEL_TYPE_SERVO;
    uint16_t canID = 0b00000000010;

    CANManager *manager = CANManager::Instance();
    manager->OnCANInit(0, canID, (uint8_t *)&msg, sizeof(msg), 0x1);
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
        delete LLController::Instance();
    }
    catch (std::exception &e)
    {
        Debug::error("signal handler: failed to shutdown LLController, %s", e.what());
    }


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
    while (true)
    {
	    sleep(1);
    }

    return 0;
}

