
#include "LLController.h"
#include "Config.h"

#include <string>
#include <iomanip>      // std::setprecision

bool running = true;

int main(int argc, char const *argv[])
{
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

