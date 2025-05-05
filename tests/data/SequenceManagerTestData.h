//
// Created by raffael on 04.05.25.
//

#ifndef SEQUENCEMANAGERTESTDATA_H
#define SEQUENCEMANAGERTESTDATA_H
#include <utility/json.hpp>
inline nlohmann::json StartIsExecutedOnlyOnce_json = R"(
{
  "globals": {
    "endTime": 1,
    "interpolation": {
      "valve_1": "inter"
    },
    "interval": 0.01,
    "startTime": -0.5
  },
  "data": [
    {
      "timestamp": "START",
      "name": "start",
      "desc": "start",
      "actions": [
        {
          "timestamp": 0.0,
          "valve_1": [
            2
          ]
        }
      ]
    },
    {
      "timestamp": "END",
      "name": "end",
      "desc": "end",
      "actions": [
        {
          "timestamp": 0.0,
          "valve_1": [
            0
          ]
        }
      ]
    }
  ]
}
)"_json;
#endif //SEQUENCEMANAGERTESTDATA_H
