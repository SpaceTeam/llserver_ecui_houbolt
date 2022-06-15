#pragma once

#include <atomic>

#include "common.h"

#include "utility/Singleton.h"


class PythonController : public Singleton<PythonController>
{
    friend class Singleton;
private:

    ~PythonController();

    std::atomic_bool running = false;
    std::thread *pyThread = nullptr;
    
public:

    void StartPythonScript(std::string scriptPath);
    void RunPyScript(std::string scriptPath);

};