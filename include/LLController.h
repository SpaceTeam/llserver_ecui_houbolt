//
// Created by Markus on 2019-10-15.
//

#ifndef TXV_ECUI_LLSERVER_LLCONTROLLER_H
#define TXV_ECUI_LLSERVER_LLCONTROLLER_H

#include "common.h"

#include "nlohmann/json.hpp"

class LLController
{

private:



    LLController();

    ~LLController();



public:

    static void Init();

    static void Destroy();

    static void OnECUISocketRecv(nlohmann::json msg);

};


#endif //TXV_ECUI_LLSERVER_LLCONTROLLER_H
