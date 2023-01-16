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
    std::vector<std::thread *> pyThreads;

    static std::string pyEnv;
    
public:

    static void SetPythonEnvironment(std::string pyEnv);

    void StartPythonScript(std::string scriptPath);
    void StartPythonScript(std::string scriptPath, std::vector<std::string> args);
    
    void RunPyScript(std::string scriptPath);
    void SetupImports();

    void RunPyScriptWithArgv(std::string scriptPath, std::vector<std::string> args);

    void RunPyScriptWithArgvWChar(std::string scriptPath, int pyArgc, wchar_t **pyArgv);

};