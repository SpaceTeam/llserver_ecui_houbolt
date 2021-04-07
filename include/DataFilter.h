//
// Created by Markus on 03.04.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_DATAFILTER_H
#define LLSERVER_ECUI_HOUBOLT_DATAFILTER_H

#include "common.h"

#include <map>
#include <string>

class DataFilter
{
private:
    double sensorsSmoothingFactor;
    std::map<std::string, std::tuple<double, uint64_t>> filteredSensorDataMap;

public:
    DataFilter(double smoothingFactor);
	~DataFilter();

	std::map<std::string, std::tuple<double, uint64_t>> FilterData(std::map<std::string, std::tuple<double, uint64_t>> &rawSensorDataMap);
};


#endif //LLSERVER_ECUI_HOUBOLT_DATAFILTER_H
