//
// Created by Markus on 05.04.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_MAPPING_H
#define LLSERVER_ECUI_HOUBOLT_MAPPING_H

#include "common.h"
#include "json.hpp"

class JSONMapping
{
protected:
    std::string mappingPath;
    nlohmann::json mapping = nullptr;


    //TODO: make thread safe
    virtual void LoadMapping();
    virtual void LoadMapping(std::string &mappingID);
    virtual void SaveMapping();

public:
    JSONMapping(std::string mappingPath);
    JSONMapping(std::string mappingPath, std::string mappingID);
    virtual ~JSONMapping();

    nlohmann::json *GetJSONMapping();
};

#endif //LLSERVER_ECUI_HOUBOLT_MAPPING_H
