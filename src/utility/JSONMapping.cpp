//
// Created by Markus on 05.04.21.
//

#include "utility/JSONMapping.h"

#include <utility/FileSystemAbstraction.h>

#include "utility/utils.h"

void JSONMapping::LoadMapping()
{

    try
    {
        Debug::print("loading mapping...");
        this->mapping = nlohmann::json::parse(FileSystemAbstraction::Instance()->LoadFile(this->mappingPath));
        if (!mappingID.empty())
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
        nlohmann::json mappingJson = nlohmann::json::parse(FileSystemAbstraction::Instance()->LoadFile(this->mappingPath));
        if (!mappingID.empty())
        {
            mappingJson[mappingID] = this->mapping;
        }
        FileSystemAbstraction::Instance()->SaveFile(this->mappingPath, mappingJson.dump(4));
        Debug::print("mapping saved");
    }
    catch(std::exception& e)
    {
        Debug::error("Mapping could not be saved: %s", e.what());
    }

}

nlohmann::json *JSONMapping::GetJSONMapping()
{
    return &this->mapping;
}

JSONMapping::JSONMapping(std::string mappingPath)
{
    JSONMapping(mappingPath, "");
}

JSONMapping::JSONMapping(std::string mappingPath, std::string mappingID)
{
    this->mappingPath = std::move(mappingPath);
    this->mappingID = std::move(mappingID);
    LoadMapping();
}

JSONMapping::~JSONMapping()
{

}



