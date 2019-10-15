
#include "LLController.h"

bool running = true;

int main(int argc, char const *argv[])
{

    LLController::Init();

    while (running){ sleep(1); }

    LLController::Destroy();

    return 0;
}

