# Low Level Server for the Engine Control User Interface

## Links

Documentation of whole ECUI and Setup Guide

### [TXV_ECUI_WEB](https://github.com/SpaceTeam/web_ecui_houbolt/tree/dev)

Temperature Sensors over Ethernet with Siliconsystems TMP01

### [TXV_ECUI_TMPoE](https://github.com/SpaceTeam/TXV_ECUI_TMPoE/tree/master)

![LLServer Diagram](img/ECUI.png)

# Notes:

- Every Sequence needs to define each device at the "START" timestamp
- **Make sure every "sensorsNominalRange" object in the sequence contains ALL sensors, with a range
specified**