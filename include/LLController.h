//
// Created by Markus on 2019-10-15.
//

#ifndef TXV_ECUI_LLSERVER_LLCONTROLLER_H
#define TXV_ECUI_LLSERVER_LLCONTROLLER_H

#include "common.h"

#include "utility/Singleton.h"

#include "utility/json.hpp"

enum ServerMode
{
	LARGE_TESTSTAND,
	SMALL_TESTSTAND,
	SMALL_OXFILL
};

class LLController : public Singleton<LLController>
{

private:

    LLInterface *llInterface;

    LLController();



    static void PrintLogo();

public:

    ~LLController();

    void Init(ServerMode serverMode);

    void Abort(std::string &abortMsg);

    void OnECUISocketRecv(nlohmann::json msg);
    void OnECUISocketClose();

};


#endif //TXV_ECUI_LLSERVER_LLCONTROLLER_H
