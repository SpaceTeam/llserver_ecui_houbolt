#ifndef SEQUENCEMANAGERTESTDATA_H
#define SEQUENCEMANAGERTESTDATA_H

#include <utility/json.hpp>
inline nlohmann::json StartIsExecutedOnlyOnce_json = R"(
{
  "globals": {
    "endTime": 1,
    "interpolation": {
      "valve_1": "none"
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

inline nlohmann::json LinearInterpolationTest1_json = R"(
{
  "globals": {
    "endTime": 1,
    "interpolation": {
      "valve_1": "linear"
    },
    "interval": 0.01,
    "startTime": 0
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
            0
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
            100
          ]
        }
      ]
    }
  ]
}
)"_json;
inline nlohmann::json SimpleAbortScenario_json = R"(
{
  "actions": {
    "valve_1": [10]
  }
}
)"_json;

inline nlohmann::json TimeStampsSwapped_json = R"({
  "globals": {
    "endTime": 1,
    "interpolation": {
      "valve_1": "none"
    },
    "interval": 0.01,
    "startTime": 0
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
            0
          ]
        }
      ]
    },
    {
      "timestamp": 0.4,
      "name": "4.0",
      "desc": "4.0",
      "actions": [
        {
          "timestamp": 0.0,
          "valve_1": [
            50
          ]
        }
      ]
    },
    {
      "timestamp": 0.2,
      "name": "2.0",
      "desc": "2.0",
      "actions": [
        {
          "timestamp": 0.0,
          "valve_1": [
            20
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
            100
          ]
        }
      ]
    }
  ]
})"_json;
#endif //SEQUENCEMANAGERTESTDATA_H
