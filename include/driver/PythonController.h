#pragma once

#include "common.h"

#include "utility/Singleton.h"


class PythonController : public Singleton<PythonController>
{
    friend class Singleton;
private:

    ~PythonController();
    
public:

    int32_t RunPyScript(std::string scriptPath);
    int32_t SetupImports();

    int32_t RunPyScriptWithArgv(std::string scriptPath, std::vector<std::string> args);

    int32_t RunPyScriptWithArgvWChar(std::string scriptPath, int pyArgc, wchar_t **pyArgv);

};