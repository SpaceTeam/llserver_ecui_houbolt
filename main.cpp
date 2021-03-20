#include <string>
#include <iomanip>      // std::setprecision
#include <sched.h>

#include <sys/stat.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "common.h"

#include "LLController.h"
#include "Config.h"

enum ServerMode
{
	LARGE_TESTSTAND,
	SMALL_TESTSTAND,
	SMALL_OXFILL
};

bool running = true;

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

int main(int argc, char const *argv[])
{
    system("clear");
    
	struct sched_param sp;
	sp.sched_priority = 60;

	sched_setscheduler(0, SCHED_FIFO, &sp);

    set_latency_target();

    ServerMode serverMode = ServerMode::LARGE_TESTSTAND;
    if (argc > 1)
    {
        if (strcmp(argv[1],"-smallTeststand") == 0)
        {
            serverMode = ServerMode::SMALL_TESTSTAND;
            printf("Using Small Teststand Profile...\n");
        }
        else if (strcmp(argv[1],"-smallOxfill") == 0)
        {
            serverMode = ServerMode::SMALL_OXFILL;
            printf("Using Small Oxfill Profile...\n");
        }
        else
        {
            printf("Defaulting to Large Teststand Profile...\n");
        }
    }

    std::string configPath = "";
    switch (serverMode)
    {
        case ServerMode::SMALL_TESTSTAND:
            configPath = "config_small_teststand.json";
			break;
        case ServerMode::SMALL_OXFILL:
            configPath = "config_small_oxfill.json";
			break;
        case ServerMode::LARGE_TESTSTAND:
            configPath = "config_large_teststand.json";
            printf("server mode 'large teststand' not implemented\n");
        default:
            exit(1);

    }
    Config::Init(configPath);

	Debug::Init();
    LLController::Init();

    std::string inputStr;
    while (true){
	    sleep(1);
    }
    std::cout << "quit" << std::endl;

    LLController::Destroy();

    return 0;
}

