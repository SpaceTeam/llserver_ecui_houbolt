//
// Created by Markus on 05.04.21.
//

#include "EventManager.h"

#include "Config.h"

EventManager::~EventManager()
{
    if (instance != nullptr)
    {
        started = false;
        initialized = false;
    }
}

LLResult EventManager::Init()
{
    if (!initialized)
    {
        Debug::print("Initializing EventMapping...");
        std::string mappingPath = std::get<std::string>(Config::getData("mapping_path"));
        mapping = new JSONMapping(mappingPath, (std::string &) "EventMapping");
        Debug::print("EventMapping initialized");
        initialized = true;
    }
    else
    {
        Debug::error("EventManager already initialized");
    }
}

LLResult EventManager::Start()
{
    if (initialized)
    {
        if (CheckEvents() == LLResult::SUCCESS)
        {
            started = true;
        }
    }
    else
    {
        Debug::error("EventManager - Start: EventManager not initialized");
    }
}

LLResult EventManager::CheckEvents()
{
    LLResult ret = LLResult::SUCCESS;

    for (auto &event : eventMap)
    {
        if (commandMap.find(event.first) != commandMap.end())
        {
            ret = LLResult::ERROR;
            Debug::error("EventManager - CheckEvents: Command '%s' not found in available commands", event.first);
        }
    }

    return ret;
}

LLResult EventManager::AddCommands(std::map<std::string, std::function<void(std::vector<double>)>> commands)
{
    LLResult ret = LLResult::ERROR;
    if (initialized)
    {
        try
        {
            commandMap.insert(commands.begin(), commands.end());
            ret = LLResult::SUCCESS;
        }
        catch (const std::exception& e)
        {
            Debug::error("EventManager - AddCommands: %s", e.what());
        }

    }
    else
    {
        Debug::error("EventManager - AddCommands: EventManager not initialized");
    }
    return ret;
}

void EventManager::OnStateChange(std::string stateName, double value)
{
    try
    {
        nlohmann::json eventJSON = eventMap[stateName];
        std::vector<double> argumentList;
        for ()

    }
    catch (const std::exception& e)
    {
        Debug::error("EventManager - OnStateChange: %s", e.what());
    }
}

