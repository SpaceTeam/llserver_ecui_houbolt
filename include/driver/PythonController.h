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

};