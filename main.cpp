
#include "LLController.h"
#include "Config.h"

#include <string>
#include <iomanip>      // std::setprecision
#include <sched.h>

bool running = true;

int main(int argc, char const *argv[])
{
	struct sched_param sp;
	sp.sched_priority = 60;

	sched_setscheduler(0, SCHED_FIFO, &sp);
    Config::Init("config.json");
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

