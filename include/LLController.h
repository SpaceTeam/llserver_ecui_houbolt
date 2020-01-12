//
// Created by Markus on 2019-10-15.
//

#ifndef TXV_ECUI_LLSERVER_LLCONTROLLER_H
#define TXV_ECUI_LLSERVER_LLCONTROLLER_H

#include "common.h"

#include "json.hpp"
#include "Config_new.h"

class LLController
{

private:



    LLController();

    ~LLController();

    static void PrintLogo();

public:

    static void Init();

    static void Destroy();

    static void Abort();

    static void OnECUISocketRecv(nlohmann::json msg);

};


#endif //TXV_ECUI_LLSERVER_LLCONTROLLER_H
