//
// Created by Markus on 07.04.21.
//

#include <DataFilter.h>

DataFilter::DataFilter(double smoothingFactor) : sensorsSmoothingFactor(smoothingFactor)
{

}

DataFilter::~DataFilter()
{

}

std::map<std::string, std::tuple<double, uint64_t>> DataFilter::FilterData(std::map<std::string, std::tuple<double, uint64_t>> &rawSensorDataMap)
{
    for (const auto& sensor : rawSensorDataMap)
    {
        double value = std::get<0>(sensor.second);
        uint64_t timestamp = std::get<1>(sensor.second);
        double *prevValue = &std::get<0>(filteredSensorDataMap[sensor.first]);
        uint64_t *prevTimestamp = &std::get<1>(filteredSensorDataMap[sensor.first]);
		if (filteredSensorDataMap.find(sensor.first) != filteredSensorDataMap.end())
		{
			filteredSensorDataMap[sensor.first] = sensor.second;
		}
		*prevValue += sensorsSmoothingFactor * (value - *prevValue);
        *prevTimestamp = timestamp;

	}

    return filteredSensorDataMap;
}
