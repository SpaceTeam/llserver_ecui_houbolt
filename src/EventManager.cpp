//
// Created by Markus on 05.04.21.
//

#include <chrono>

#include "EventManager.h"

#include "StateController.h"

#include "utility/Config.h"

EventManager::~EventManager()
{
    if (instance != nullptr)
    {
        started = false;
        initialized = false;
    }
}

void EventManager::Init()
{
    if (!initialized)
    {
        Debug::print("Initializing EventManager...");
        try
        {
            mapping = new JSONMapping(Config::getMappingFilePath(), "EventMapping");
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

void EventManager::ExecuteCommand(const std::string &stateName, double oldValue, double newValue, bool testOnly)
{
    if (!mappingJSON.contains(stateName)){
        //state name not in mapping, shall not trigger anything
        Debug::info("EventManager - OnStateChange: state name not in event mapping, ignored...");
        return;
    }
    nlohmann::json events = mappingJSON[stateName];
    for (auto& eventJSON : events)
    {
        if (eventJSON.contains("triggerType"))
        {
            std::string triggerType = eventJSON["triggerType"];
            double triggerValue = eventJSON["triggerValue"];

            //Lo and Behold, this actually works! Why you ask my friend... well
            //https://www.cplusplus.com/reference/string/string/operators/ says relational operators
            //use operator overloading to compare two strings using the compare method.
            //C++ is a mighty language right? What do you think?
            if (triggerType == "==")
            {
                if (newValue != triggerValue)
                {
                    Debug::info("EventManager - OnStateChange: trigger type %s | "
                                "state value %d unequal to event mapping value %d, ignored...",
                                triggerType.c_str(), newValue, triggerValue);
                    continue;
                }
                else
                {
                    //old value also in trigger range, do not trigger
                    if (oldValue == triggerValue)
                    {
                        continue;
                    }
                }
            }
            else if (triggerType == "!=")
            {
                if (newValue == triggerValue)
                {
                    Debug::info("EventManager - OnStateChange: trigger type %s | "
                                "state value %d equal to event mapping value %d, ignored...",
                                triggerType.c_str(), newValue, triggerValue);
                    continue;
                }
                else
                {
                    //old value also in trigger range, do not trigger
                    if (oldValue != triggerValue)
                    {
                        continue;
                    }
                }
            }
            else if (triggerType == ">=")
            {
                if (newValue < triggerValue)
                {
                    Debug::info("EventManager - OnStateChange: trigger type %s | "
                                "state value %d smaller than event mapping value %d, ignored...",
                                triggerType.c_str(), newValue, triggerValue);
                    continue;
                }
                else
                {
                    //old value also in trigger range, do not trigger
                    if (oldValue >= triggerValue)
                    {
                        continue;
                    }
                }
            }
            else if (triggerType == "<=")
            {
                if (newValue > triggerValue)
                {
                    Debug::info("EventManager - OnStateChange: trigger type %s | "
                                "state value %d greater than event mapping value %d, ignored...",
                                triggerType.c_str(), newValue, triggerValue);
                    continue;
                }
                else
                {
                    //old value also in trigger range, do not trigger
                    if (oldValue <= triggerValue)
                    {
                        continue;
                    }
                }
            }
            else if (triggerType == ">")
            {
                if (newValue <= triggerValue)
                {
                    Debug::info("EventManager - OnStateChange: trigger type %s | "
                                "state value %d smaller or equal than event mapping value %d, ignored...",
                                triggerType.c_str(), newValue, triggerValue);
                    continue;
                }
                else
                {
                    //old value also in trigger range, do not trigger
                    if (oldValue > triggerValue)
                    {
                        continue;
                    }
                }
            }
            else if (triggerType == "<")
            {
                if (newValue >= triggerValue)
                {
                    Debug::info("EventManager - OnStateChange: trigger type %s | "
                                "state value %d greater or equal than event mapping value %d, ignored...",
                                triggerType.c_str(), newValue, triggerValue);
                    continue;
                }
                else
                {
                    //old value also in trigger range, do not trigger
                    if (oldValue < triggerValue)
                    {
                        continue;
                    }
                }
            }
        }
        else
        {
            Debug::info("EventManager - OnStateChange: no trigger type found, always triggering...");
        }

        std::vector<double> argumentList;
        for (nlohmann::json &param : eventJSON["parameters"])
        {

            if (param.is_string())
            {
                if (stateName.compare(param) == 0)
                {
                    argumentList.push_back(newValue);
                }
                else
                {
                    StateController *controller = StateController::Instance();
                    double val = controller->GetStateValue(param);
                    argumentList.push_back(val);
                }
            }
            else if (param.is_number())
            {
                argumentList.push_back(param);
            }
            else
            {
                throw std::invalid_argument( "parameter not string or number" );
            }
        }

        std::string commandName = eventJSON["command"];
        //trigger command
        if (commandMap.find(commandName) == commandMap.end()){
            //state name not in mapping, shall not trigger anything
            throw std::runtime_error("command " + commandName + " not implemented");
            continue;
        }
        auto commandFunc = std::get<0>(commandMap[commandName]);
        commandFunc(argumentList, testOnly);
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
        ExecuteCommand(stateName, oldValue, newValue, false);
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
