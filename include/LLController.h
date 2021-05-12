//
// Created by Markus on 2019-10-15.
//

#ifndef TXV_ECUI_LLSERVER_LLCONTROLLER_H
#define TXV_ECUI_LLSERVER_LLCONTROLLER_H

#include "common.h"
#include <atomic>

#include "LLInterface.h"
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
    friend class Singleton;
private:

    LLInterface *llInterface;

    std::atomic_bool initialized = false;

    static void PrintLogo();

    ~LLController();
public:

    void Init(ServerMode serverMode);

    bool IsInitialized();

    void Abort(std::string &abortMsg);

    void OnECUISocketRecv(nlohmann::json msg);
    void OnECUISocketClose();

};


#endif //TXV_ECUI_LLSERVER_LLCONTROLLER_H
