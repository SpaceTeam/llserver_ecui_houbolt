
#include "LLController.h"

#include <string>
#include <iomanip>      // std::setprecision

bool running = true;

int main(int argc, char const *argv[])
{
    LLController::Init();

    std::string inputStr;
    while (getline (std::cin, inputStr)){
    }
    std::cout << "quit" << std::endl;

    LLController::Destroy();

    return 0;
}

