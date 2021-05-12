//
// Created by Markus on 05.04.21.
//

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
            std::string mappingPath = std::get<std::string>(Config::getData("mapping_path"));
            mapping = new JSONMapping(mappingPath, "EventMapping");
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
            Debug::error("EventManager - CheckEvents: Command '%s' not found in available commands", event.first);

            //TODO: MP call command but with argument flag to only check parameters
            return false;
        }
    }

    return true;
}

void EventManager::AddCommands(std::map<std::string, std::function<void(std::vector<double> &, bool)>> commands)
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

/**
 * when exception occurs the error is only logged but nothing else happens
 * @param stateName
 * @param value
 */
void EventManager::OnStateChange(const std::string& stateName, double value)
{
    try
    {
        if (!mappingJSON.contains(stateName)){
            //state name not in mapping, shall not trigger anything
            Debug::info("EventManager - OnStateChange: state name not in event mapping, ignored...");
            return;
        }
        nlohmann::json eventJSON = mappingJSON[stateName];
        if (eventJSON.contains("=="))
        {
            if (value != eventJSON["=="])
            {
                Debug::info("EventManager - OnStateChange: state value %d unequal to event mapping value %d, ignored...", value, (double)eventJSON["=="]);
                return;
            }
        }
        std::vector<double> argumentList;
        for (nlohmann::json &param : eventJSON["parameters"])
        {

            if (param.is_string())
            {
                if (stateName.compare(param) == 0)
                {
                    argumentList.push_back(value);
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
        nlohmann::json command = eventJSON["command"];
        //trigger command
        auto commandFunc = commandMap[command];
        commandFunc(argumentList, false);
    }
    catch (const std::exception& e)
    {
        Debug::error("EventManager - OnStateChange: %s", e.what());
    }
}

