//
// Created by Markus on 31.03.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_EVENTMANAGER_H
#define LLSERVER_ECUI_HOUBOLT_EVENTMANAGER_H

#include "common.h"

#include "utility/json.hpp"
#include "utility/Config.h"

#include <map>
#include <functional>
#include <vector>
#include <string>

#include "utility/Singleton.h"
#include "utility/JSONMapping.h"

/**
 * std::function pointer to function for command expects double parameter list and bool (testOnly)
 * std::vector<std::string> list of parameter names in its order
 */
typedef std::tuple<std::function<void(std::vector<double> &, bool)>, std::vector<std::string>> command_t;

//change event mapping when sequence is running to still be able to throw events when sequence is running (or just disable)

class EventManager : public Singleton<EventManager>
{
    friend class Singleton;
private:

    bool initialized = false;
    bool started = false;
    //TODO: write channel cmds as method in each channel class
    //<stateName, <callback, pointer to state>
    std::map<std::string, command_t> eventMap;
    std::map<std::string, std::string> channelTypeMap; //1. stateName, 2. stateType
    std::map<std::string, command_t> commandMap;

    JSONMapping *defaultMapping;
    nlohmann::json defaultMappingJSON;

    JSONMapping *mapping;
    nlohmann::json mappingJSON;

    bool CheckEvents();

    bool ShallTrigger(nlohmann::json& event, double& oldValue, double& newValue);
    double GetArgument(const std::string &stateName, nlohmann::json& param, double& newValue);
    void GetArgumentList(const std::string &stateName, nlohmann::json& event, std::vector<double>& argumentList, double& newValue);

public:
    virtual void Init(Config &config);

    /**
     * this is used, so mapping and command loading don't need to be done at the same time,
     * this way, the event manager doesn't need to call any classes below the llinterface
     */
    virtual void Start();

    virtual void AddChannelTypes(std::map<std::string, std::string>& channelTypes);

    virtual void AddCommands(std::map<std::string, command_t> commands);

    virtual std::map<std::string, command_t> GetCommands();

    virtual void OnStateChange(const std::string& stateName, double oldValue, double newValue);

    virtual void ExecuteRegexCommandOrState(const std::string &regexKey, nlohmann::json &events, const std::string &stateName, double oldValue, double newValue, bool testOnly);
    virtual void ExecuteCommandOrState(const std::string &stateName, double oldValue, double newValue, bool useDefaultMapping, bool testOnly);
    virtual void ExecuteCommand(const std::string &commandName, std::vector<double> &params, bool testOnly);

    virtual ~EventManager();

};
#endif //LLSERVER_ECUI_HOUBOLT_EVENTMANAGER_H
