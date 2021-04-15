//
// Created by Markus on 31.03.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_EVENTMANAGER_H
#define LLSERVER_ECUI_HOUBOLT_EVENTMANAGER_H

#include "common.h"

#include "utility/json.hpp"

#include <map>
#include <functional>
#include <vector>
#include <string>

#include "utility/Singleton.h"
#include "drivers/JSONMapping.h"

//change event mapping when sequence is running to still be able to throw events when sequence is running (or just disable)

class EventManager : public Singleton<EventManager>
{
private:

    bool initialized = false;
    bool started = false;
    //TODO: write channel cmds as method in each channel class
    //<stateName, <callback, pointer to state>
    std::map<std::string, std::function<void(std::vector<double> &, bool)>> eventMap;
    std::map<std::string, std::function<void(std::vector<double> &, bool)>> commandMap;

    JSONMapping *mapping;
    nlohmann::json mappingJSON;

    bool CheckEvents();

public:
    ~EventManager();

    void Init();

    /**
     * this is used, so mapping and command loading don't need to be done at the same time,
     * this way, the event manager doesn't need to call any classes below the llinterface
     */
    void Start();

    void AddCommands(std::map<std::string, std::function<void(std::vector<double> &, bool)>> commands);
    void OnStateChange(const std::string& stateName, double value);


};
#endif //LLSERVER_ECUI_HOUBOLT_EVENTMANAGER_H
