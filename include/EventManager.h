//
// Created by Markus on 31.03.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_EVENTMANAGER_H
#define LLSERVER_ECUI_HOUBOLT_EVENTMANAGER_H

#include "common.h"

#include <map>
#include <functional>

//change event mapping when sequence is running to still be able to throw events when sequence is running (or just disable)

class EventManager
{
private:
    //TODO: write channel cmds as method in each channel class
    //<stateName, <callback, pointer to state>
    static std::map<std::string, std::function<bool(...)>> eventMap;

	EventManager();
	~EventManager();
public:
	static LLResult Init();

	static void OnStateChange(std::string stateName, double value);
};
#endif //LLSERVER_ECUI_HOUBOLT_EVENTMANAGER_H
