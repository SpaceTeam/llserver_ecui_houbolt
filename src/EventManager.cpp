//
// Created by Markus on 05.04.21.
//

#include <chrono>
#include <regex>

#include "EventManager.h"

#include "StateController.h"

#include "utility/utils.h"

EventManager::~EventManager()
{
    if (instance != nullptr)
    {
        started = false;
        initialized = false;
    }
}

void EventManager::Init(Config &config)
{
    if (!initialized)
    {
        Debug::print("Initializing EventManager...");
        try
        {
            defaultMapping = new JSONMapping(config.getMappingFilePath(), "DefaultEventMapping");
            defaultMappingJSON = *defaultMapping->GetJSONMapping();
            Debug::print("DefaultEventMapping initialized");

            mapping = new JSONMapping(config.getMappingFilePath(), "EventMapping");
            mappingJSON = *mapping->GetJSONMapping();
            Debug::print("EventMapping initialized");
            initialized = true;
        }
        catch (std::exception& e)
        {
            Debug::error("Initializing EventdManager failed: %s", e.what());
        }
    }
    else
    {
        Debug::error("EventManager already initialized");
    }
}

void EventManager::Start()
{
    if (initialized)
    {
        if (CheckEvents())
        {
            started = true;
        }
    }
    else
    {
        Debug::error("EventManager - Start: EventManager not initialized");
    }
}

bool EventManager::CheckEvents()
{

    for (auto &event : eventMap)
    {
        if (commandMap.find(event.first) == commandMap.end())
        {
            Debug::error("EventManager - CheckEvents: Command '%s' not found in available commands", ((std::string)(event.first)).c_str());


            return false;
        }
        //TODO: MP call command but with argument flag to only check parameters
    }

    return true;
}

bool EventManager::ShallTrigger(nlohmann::json& event, double& oldValue, double& newValue)
{
    bool shallTrigger = true;
    if (event.contains("triggerType"))
        {
            std::string triggerType = event["triggerType"];
            double triggerValue = event["triggerValue"];

            //Lo and Behold, this actually works! Why you ask my friend... well
            //https://www.cplusplus.com/reference/string/string/operators/ says relational operators
            //use operator overloading to compare two strings using the compare method.
            //C++ is a mighty language right? What do you think?
            if (triggerType == "==")
            {
                if (newValue != triggerValue)
                {
                    Debug::info("EventManager - ShallTrigger: trigger type %s | "
                                "state value %d unequal to event mapping value %d, ignored...",
                                triggerType.c_str(), newValue, triggerValue);
                    shallTrigger = false;
                }
                else
                {
                    //old value also in trigger range, do not trigger
                    if (oldValue == triggerValue)
                    {
                        shallTrigger = false;
                    }
                }
            }
            else if (triggerType == "!=")
            {
                if (newValue == triggerValue)
                {
                    Debug::info("EventManager - ShallTrigger: trigger type %s | "
                                "state value %d equal to event mapping value %d, ignored...",
                                triggerType.c_str(), newValue, triggerValue);
                    shallTrigger = false;
                }
                else
                {
                    //old value also in trigger range, do not trigger
                    if (oldValue != triggerValue && !std::isnan(oldValue))
                    {
                        shallTrigger = false;
                    }
                }
            }
            else if (triggerType == ">=")
            {
                if (newValue < triggerValue)
                {
                    Debug::info("EventManager - ShallTrigger: trigger type %s | "
                                "state value %d smaller than event mapping value %d, ignored...",
                                triggerType.c_str(), newValue, triggerValue);
                    shallTrigger = false;
                }
                else
                {
                    //old value also in trigger range, do not trigger
                    if (oldValue >= triggerValue)
                    {
                        shallTrigger = false;
                    }
                }
            }
            else if (triggerType == "<=")
            {
                if (newValue > triggerValue)
                {
                    Debug::info("EventManager - ShallTrigger: trigger type %s | "
                                "state value %d greater than event mapping value %d, ignored...",
                                triggerType.c_str(), newValue, triggerValue);
                    shallTrigger = false;
                }
                else
                {
                    //old value also in trigger range, do not trigger
                    if (oldValue <= triggerValue)
                    {
                        shallTrigger = false;
                    }
                }
            }
            else if (triggerType == ">")
            {
                if (newValue <= triggerValue)
                {
                    Debug::info("EventManager - ShallTrigger: trigger type %s | "
                                "state value %d smaller or equal than event mapping value %d, ignored...",
                                triggerType.c_str(), newValue, triggerValue);
                    shallTrigger = false;
                }
                else
                {
                    //old value also in trigger range, do not trigger
                    if (oldValue > triggerValue)
                    {
                        shallTrigger = false;
                    }
                }
            }
            else if (triggerType == "<")
            {
                if (newValue >= triggerValue)
                {
                    Debug::info("EventManager - ShallTrigger: trigger type %s | "
                                "state value %d greater or equal than event mapping value %d, ignored...",
                                triggerType.c_str(), newValue, triggerValue);
                    shallTrigger = false;
                }
                else
                {
                    //old value also in trigger range, do not trigger
                    if (oldValue < triggerValue)
                    {
                        shallTrigger = false;
                    }
                }
            }
        }
        else
        {
            Debug::info("EventManager - ShallTrigger: no trigger type found, always triggering...");
        }
    return shallTrigger;
}

double EventManager::GetArgument(const std::string &stateName, nlohmann::json& param, double& newValue)
{
    if (param.is_string())
    {
        if (stateName.compare(param) == 0)
        {
            return newValue;
        }
        else
        {
            StateController *controller = StateController::Instance();
            double val = controller->GetStateValue(param);
            return val;
        }
    }
    else if (param.is_number())
    {
        return param;
    }
    else
    {
        throw std::invalid_argument( "EventManager - GetArgument: parameter not string or number" );
    }
}
    
void EventManager::GetArgumentList(const std::string &stateName, nlohmann::json& event, std::vector<double>& argumentList, double& newValue)
{
    for (nlohmann::json &param : event["parameters"])
    {
        argumentList.push_back(GetArgument(stateName, param, newValue));
    }
}

void EventManager::AddChannelTypes(std::map<std::string, std::string>& channelTypes)
{
    if (initialized)
    {
        try
        {
            channelTypeMap.insert(channelTypes.begin(), channelTypes.end());
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error("EventManager - AddChannelTypes: " + std::string(e.what()));
        }

    }
    else
    {
        throw std::runtime_error("EventManager - AddChannelTypes: EventManager not initialized");
    }
}

void EventManager::AddCommands(std::map<std::string, command_t> commands)
{
    if (initialized)
    {
        try
        {
            commandMap.insert(commands.begin(), commands.end());
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error("EventManager - AddCommands: " + std::string(e.what()));
        }

    }
    else
    {
        throw std::runtime_error("EventManager - AddCommands: EventManager not initialized");
    }
}

std::map<std::string, command_t> EventManager::GetCommands()
{
    return commandMap;
}

void EventManager::ExecuteRegexCommandOrState(const std::string &regexKey, nlohmann::json &events, const std::string &stateName, double oldValue, double newValue, bool testOnly)
{
    auto stateSplit = utils::split(stateName, ":");
    stateSplit.pop_back();
    auto strippedState = utils::merge(stateSplit, ":");

    for (auto& eventJSON : events)
    {
        if (!ShallTrigger(eventJSON, oldValue, newValue))
        {
            continue;
        }

        bool isState = utils::keyExists(eventJSON, "state");
        bool isCommand = utils::keyExists(eventJSON, "command");
        if (isState && isCommand)
        {
            throw std::invalid_argument( "EventManager - ExecuteRegexCommandOrState: can't have both command and state key in event definiton" );
        }
        else if (isState)
        {
            double newStateVal;
            if (utils::keyExists(eventJSON, "value"))
            {
                nlohmann::json param = eventJSON["value"];
                if (regexKey == eventJSON["value"])
                {
                    param = stateName;
                }
                newStateVal = GetArgument(stateName, param, newValue);
                StateController::Instance()->SetState(utils::replace(eventJSON["state"], "{}", strippedState), newStateVal, utils::getCurrentTimestamp());
            }
            else
            {
                throw std::invalid_argument( "EventManager - ExecuteRegexCommandOrState: need value key when specifying state in event definiton" );
            }
        }
        else if (isCommand)
        {
            std::vector<double> argumentList;
            for (nlohmann::json &param : eventJSON["parameters"])
            {
                if (regexKey == param)
                {
                    param = stateName;
                }
            }
            GetArgumentList(stateName, eventJSON, argumentList, newValue);

            std::string commandName = utils::replace(eventJSON["command"], "{}", strippedState);

            //trigger command
            if (commandMap.find(commandName) == commandMap.end()){
                //state name not in mapping, shall not trigger anything
                Debug::error("EventManager - ExecuteRegexCommandOrState: " + commandName + " not implemented, ignoring...");
                continue;
            }
            auto commandFunc = std::get<0>(commandMap[commandName]);
            commandFunc(argumentList, testOnly);
        }
        else
        {
            throw std::invalid_argument( "EventManager - ExecuteRegexCommandOrState: need either command or state key in event definiton" );
        }


        
    }

}

void EventManager::ExecuteCommandOrState(const std::string &stateName, double oldValue, double newValue, bool useDefaultMapping, bool testOnly)
{
    nlohmann::json events;
    std::string currStateName = stateName;
    std::string commandChannelName = stateName;
    if (useDefaultMapping)
    {
        utils::replaceRef(commandChannelName, "gui:", "");
        utils::replaceRef(commandChannelName, ":sensor", "");
        currStateName = channelTypeMap[commandChannelName];
        if (defaultMappingJSON.contains(currStateName))
        {
            Debug::info("Found default event for: %s", stateName);
        }
        events = defaultMappingJSON[currStateName];
    }
    else
    {
        events = mappingJSON[currStateName];
    }
    
    for (auto& eventJSON : events)
    {
        if (!ShallTrigger(eventJSON, oldValue, newValue))
        {
            continue;
        }

        bool isState = utils::keyExists(eventJSON, "state");
        bool isCommand = utils::keyExists(eventJSON, "command");
        if (isState && isCommand)
        {
            throw std::invalid_argument( "EventManager - ExecuteCommandOrState: can't have both command and state key in event definiton" );
        }
        else if (isState)
        {
            double newStateVal;
            if (utils::keyExists(eventJSON, "value"))
            {
                newStateVal = GetArgument(stateName, eventJSON["value"], newValue);
                StateController::Instance()->SetState(eventJSON["state"], newStateVal, utils::getCurrentTimestamp());
            }
            else
            {
                throw std::invalid_argument( "EventManager - ExecuteCommandOrState: need value key when specifying state in event definiton" );
            }
        }
        else if (isCommand)
        {
            std::vector<double> argumentList;
            GetArgumentList(currStateName, eventJSON, argumentList, newValue);

            std::string commandName = eventJSON["command"];
            if (useDefaultMapping)
            {
                utils::replaceRef(commandName, currStateName, commandChannelName);
            }
            //trigger command
            if (commandMap.find(commandName) == commandMap.end()){
                //state name not in mapping, shall not trigger anything
                Debug::error("EventManager - ExecuteCommandOrState: " + commandName + " not implemented, ignoring...");
                continue;
            }
            auto commandFunc = std::get<0>(commandMap[commandName]);
            commandFunc(argumentList, testOnly);
        }
        else
        {
            throw std::invalid_argument( "EventManager - ExecuteCommandOrState: need either command or state key in event definiton" );
        }


        
    }
}

/**
 * when an exception occurs the error is only logged but nothing else happens
 * @param stateName
 * @param value
 */
void EventManager::OnStateChange(const std::string& stateName, double oldValue, double newValue)
{
    try
    {
        if (!mappingJSON.contains(stateName))
        {
            bool foundMatch = false;
            /* for (auto it = mappingJSON.begin(); it != mappingJSON.end(); ++it)
            {
                std::string keyName = it.key();
                std::regex regex(keyName);
                if (std::regex_match(stateName, regex))
                {
                    ExecuteRegexCommandOrState(it.key(), it.value(), stateName, oldValue, newValue, false);
                    foundMatch = true;
                }
            } */
            if (!foundMatch && (stateName.find("gui:") != std::string::npos)) {
                Debug::info("EventManager - OnStateChange: State name not found, looking for default config...");
                ExecuteCommandOrState(stateName, oldValue, newValue, true, false);
            }
            else
            {
                //Debug::info("EventManager - OnStateChange: State name not in event mapping...");
            }    
        }
        else
        {
            ExecuteCommandOrState(stateName, oldValue, newValue, false, false);
        }
        
    }
    catch (const std::exception& e)
    {
        Debug::error("EventManager - OnStateChange: %s", e.what());
    }
}

void EventManager::ExecuteCommand(const std::string &commandName, std::vector<double> &params, bool testOnly)
{
    if (commandMap.find(commandName) == commandMap.end()){
        //state name not in mapping, shall not trigger anything
        throw std::runtime_error("command " + commandName + " not implemented");
        return;
    }
    auto commandFunc = std::get<0>(commandMap[commandName]);
    commandFunc(params, testOnly);
}
