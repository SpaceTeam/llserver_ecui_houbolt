//
// Created by Markus on 05.04.21.
//

#include "drivers/JSONMapping.h"

#include "utils.h"

void JSONMapping::LoadMapping()
{
    LoadMapping((std::string &) "");
}

void JSONMapping::LoadMapping(std::string &mappingID)
{

    try
    {
        Debug::print("loading mapping...");
        this->mapping = nlohmann::json::parse(utils::loadFile(this->mappingPath));
        if (mappingID.empty())
        {
            this->mapping = this->mapping[mappingID];
        }
        Debug::print("mapping loaded");
    }
    catch(std::exception& e)
    {
        this->mapping = nullptr;
        Debug::error("Mapping could not be loaded: %s", e.what());
    }

}

void JSONMapping::SaveMapping()
{
    try
    {
        Debug::print("saving mapping...");
        utils::saveFile(this->mappingPath, mapping.dump(4));
        Debug::print("mapping saved");
    }
    catch(std::exception& e)
    {
        Debug::error("Mapping could not be saved: %s", e.what());
    }

}

nlohmann::json JSONMapping::GetJSONMapping()
{
    return this->mapping;
}

JSONMapping::JSONMapping(std::string mappingPath)
{
    this->mappingPath = std::move(mappingPath);
    LoadMapping();
}

JSONMapping::JSONMapping(std::string mappingPath, std::string mappingID)
{
    this->mappingPath = std::move(mappingPath);
    LoadMapping(mappingID);
}

JSONMapping::~JSONMapping()
{

}



