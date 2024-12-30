<h1>Low Level Server for the SpaceTeam Mission Control System</h1>

<h2>Table of Contents</h2>


- [Overview](#overview)
- [Requirements](#requirements)
- [Installation](#installation)
  - [Build options](#build-options)
  - [Supported CAN Drivers](#supported-can-drivers)
- [Development Setup](#development-setup)
- [CAN Protocol](#can-protocol)
- [The importance of States](#the-importance-of-states)
- [Events](#events)
  - [State to CAN Command](#state-to-can-command)
  - [State to State](#state-to-state)
  - [Trigger Types](#trigger-types)
- [Configuration](#configuration)
  - [config.json](#configjson)
  - [mapping.json](#mappingjson)
    - [CANMapping](#canmapping)
    - [DefaultEventMapping](#defaulteventmapping)
    - [EventMapping](#eventmapping)
    - [GUIMapping](#guimapping)
- [Autonomous Control Test Sequence](#autonomous-control-test-sequence)
  - [`globals` section](#globals-section)
  - [`data` section](#data-section)
  - [Abort Sequence Format](#abort-sequence-format)
- [TCP Socket Message Types](#tcp-socket-message-types)
- [UDP Socket Endpoint for LoRa](#udp-socket-endpoint-for-lora)
  - [LoRa Config](#lora-config)
- [Troubleshooting](#troubleshooting)

## Overview

The SpaceTeam Mission Control System (STMC) Suite consists of multiple programms that
form a system for Monitoring and Remote Control Purposes. Historically it was developed
for Testing Rocket Engines and has then been further extended to be usable as a MissionControl
Interface. For further information follow [SpaceTeam Mission Control System](...)

**The Low Level Server is responsible for handling and processing all time critical tasks**

This includes the implementation of 
- our own [CAN Protocol](https://github.com/SpaceTeam/can_houbolt)
- a possible UDP endpoint for using our CAN Protocol in an optimized way using
  our custom LoRa shield with a custom LoRa Driver (will get public soon)
- the Database Interface (influxDB) 
- the Control Sequence Logic
- the Interface to our own [Webserver](https://github.com/SpaceTeam/web_ecui_houbolt) 
  
The complexity of this program lies in the various configurable
options for initialization and during runtime. They are split into two files that 
**must** must reside in the same directory:

- config.json - the config file for socket endpoints, sampling rates, CAN bus params, etc.
- mapping.json 
  - [CANMapping](#canmapping)
  - [DefaultEventMapping](#defaulteventmapping)
  - [EventMapping](#eventmapping)
  - [GUIMapping](#guimapping)

In our setup these files can be found in [config_ecui](https://github.com/SpaceTeam/config_ecui), but this is not mandatory.
The config file directory path can be set either by passing it on start as an argument
or by setting the environment variable **ECUI_CONFIG_PATH**. The argument has
priority over the environment variable.

## Requirements

>Note: If you would like to use the llserver out of the box you also need to
install our webserver and a bunch of other tools. But don't worry, **we've written
an easy to setup guide in our [config_ecui](https://github.com/SpaceTeam/config_ecui)
repository.** Otherwise you first need to implement a tcp server with our communication
protocol in order to start receiving data.

The simplest way to use the llserver is by setting it up inside a docker container
in that case docker engine needs to be preinstalled. 

First you need to install an influxdb (docker container recommended)

## Installation

To install the llserver using docker you can run 
```
sudo chmod +x install.sh
./install.sh
```

This script generates and mounts a config folder in the parent directory named
config_ecui where the [config.json](#configjson) and [mapping.json](#mappingjson) files must be present.

### Build options
The project uses cmake for compiling. You can provide different build options to
cmake using the `-D` option:

- `-D NO_PYTHON=[true | false]`
- `-D NO_CANLIB=[true | false]`

Setting one of these build flags result in not binding the corresponding libraries
to avoid a compile error. If you would like to change one of these settings checkout
the Entrypoint line in the Dockerfile 

>NOTE: different build modes require different config variables, make sure you
have definded all of them properly. See [config_ecui](https://github.com/SpaceTeam/config_ecui).

### Supported CAN Drivers
Currently two drivers are supported namely
- [Kvaser CANLIB](https://www.kvaser.com/canlib-webhelp/page_canlib.html)
- [SocketCAN](https://docs.kernel.org/networking/can.html)
  
the preferred driver can be selected inside the [config.json](#configjson).
The Dockerfile also installs all required kvaser canlib library files automatically.

## Development Setup
The LLServer is meant to be used in conjuction with a can-bus and an influxdb logging server.
For local development the following setup is recommended:

### can interface
The LLServer still requires a can interface to be present on the system to function.
You can create a dummy can interface with the following commands:
```bash
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0
```

### Building the LLServer
Use the following cmake variables to run the LLServer in Dev Mode:
```bash
mkdir build && cd build
cmake -D NO_PYTHON=true -D NO_CANLIB=true -D NO_INFLUX=true .. -o ./
make -j
```
This disables the python bindings, the canbus interface and the influxdb logging.

### Running the LLServer
The LLServer requires a `config.json` and a `mapping.json` file to run. You can find examples in the `config` folder.

To run the LLServer with the example config files, use the following command:
```bash
 ECUI_CONFIG_PATH=$(dirname "$PWD")/sample_config ./llserver_ecui_houbolt
```

Please note that without the CAN Bus none of the commands will work. You can crudely some for testing like this in the LLInterface::init:

```c++
        auto doStuff = [](std::vector<double> &data, bool flag) {
            std::string str = [&data] {
                std::ostringstream oss;
                std::copy(data.begin(), data.end(), std::ostream_iterator<double>(oss, " "));
                return oss.str();
            }();
            Debug::print("doStuff flag: %d values: %s", flag, str.c_str());
        };

        eventManager->AddCommands({{"doStuff", {doStuff, {}}}});

```



### Emulating the ECUI
The LLServer communicates over a TCP-Socket with the [ECUI](https://github.com/SpaceTeam/web_ecui_houbolt).
To emulate this one can use the [ECUIEmulator.py](scripts/ECUIEmulator.py) script.
You have to set the message `type` and `data` and it will send it to the LLServer.
Responses are logged to the console.

Eg. Starting a sequence:
```bash
python3 scripts/ECUIEmulator.py
Starting JSON socket server.
Enter your message type(one of sequence-start, send-postseq-comment, abort, auto-abort-change, states-load, states-get, states-set, states-start, states-stop, gui-mapping-load, commands-load, commands-set):
sequence-start
Enter your JSON message (send with `END` in a new line):
{
  "globals": {
    "endTime": 10,
    "interpolation": {
      "doStuff": "none"
    },
    "interval": 0.01,
    "startTime": -3
  },
  "data": [
    {
      "timestamp": "START",
      "name": "start",
      "desc": "start",
      "actions": [
        {
          "timestamp": 0.0,
          "doStuff": [
            2
          ]
        }
      ]
    }
    
  ]
}
END
Message sent.

```

## CAN Protocol
As many terms from the [CAN Protocol](https://github.com/SpaceTeam/can_houbolt) 
are also used to describe many functionalities 
in the llserver, it is recommended to read through this documentation first before
you continue.

The llserver keeps a hashmap of all commands that are supported of the connected
channels. These are used in the event mapping and control sequences to
send commands to the hardware.

## The importance of States
The llserver manages a hashmap filled with states which is called the
**State Controller**. Basically every variable inside the system is represented 
as a double value inside the state controller. This includes user inputs.

Each entry consists of a

- state name - as key
- value - as double
- timestamp - 0 when uninitialized else timestamp of the last change
- dirty flag - used for transmitting periodic status updates to the web server

We follow a naming convention which mostly boils down to

`prefix:channel_name:state`

as the `prefix` it is often used `gui` as all user inputs are also tracked in the
state controller and logged to the database. In case if something goes wrong we can 
analyze whether a wrong user input was made or another event caused the trouble.
**User inputs must be prefixed with `gui` in order to be processed correctly.**

Despite our naming convetion of state names, one can input a state name
with as many `":"` dividers as desired. 

>WARNING: Although a `:` is used to make the state names more readable and
processable it shall be pointed out that this notation is not supported in 
html. Hence all `:` are translated to `-` when received at the web client.

## Events
Another key feature of the llserver is the event system. An event can be triggered
when a specific state changes its value. This is used for translating user inputs
to CAN commands that are transmitted over the CAN bus for example. But it is also
possible to trigger an event based on a sensor value or actuator state
of a specific hardware channel (since they
are all represented as states). For this behaviour to work there are two types of events

### State to CAN Command

```
"gui:Flash:Clear": [
    {
        "command": "ecu:RequestFlashClear",
        "parameters": []
    },
    {
        "command": "pmu:RequestFlashClear",
        "parameters": []
    },
    {
        "command": "rcu:RequestFlashClear",
        "parameters": []
    }
]
```
In this example the `gui:Flash:Clear` state indicates a button press on the user interface. When it is pressed, a request to clear the flash of each electronics board
(ecu, pmu, rcu) shall be transmitted. The `parameters` entry allows for additional
arguments that may be required depending on the specified command. When
a string is located inside the parameters list, the llserver tries to resolve it
as a state inside the state controller and use its value instead. This is especially
useful for analog user inputs.

### State to State
```
"ecu:FlashStatus": [
    {
        "state": "gui:Flash:Clear",
        "triggerType": "!=",
        "triggerValue": 0,
        "value": 0
    }       	
]
```

In bot cases it is possible to trigger multiple actions in one go as seen in the
example of `gui:Flash:Clear`. Also in both cases it is possible to specify a triggerType.

### Trigger Types
Each event entry can include a `triggerType` with an additional `triggerValue`. 
This is needed if the event shall only
be triggered when certain conditions are met, not everytime the state gets changed.
Possible trigger types are:

- `==` --> triggers when the state value equals the `triggerValue`
- `!=` --> triggers when the state value not equals the `triggerValue`
- `>=` --> triggers when the state value is greater or equal than the `triggerValue`
- `<=` --> triggers when the state value is smaller or equal than the `triggerValue`
- `>` --> triggers when the state value greater than the `triggerValue`
- `<` --> triggers when the state value smaller than the `triggerValue`

>NOTE: An event with a triggerType is only triggered when the previous value
was outside the trigger range! In this way the event only gets triggered when
the value changes from outside the trigger range to the inside of the trigger range which
means that if the value remains inside the trigger range over a long period the event
only gets triggered once. 

Example:
```
"gui:fuel_main_valve:checkbox": [
    {
        "command": "fuel_main_valve:SetTargetPosition",
        "triggerType": "==",
        "triggerValue": 0,
        "parameters": [0]
    },
    {
        "command": "fuel_main_valve:SetTargetPosition",
        "triggerType": "!=",
        "triggerValue": 0,
        "parameters": [65535]
    }
]
```
In this case the gui element for the fuel main valve is represented as a checkbox.
when the checkbox gets pressed, the fuel main valve opens completely. Otherwise
the valve closes completely. When multiple actions per state exist, the program
processes them step by step, as defined in the json array. **So be reminded that
a different order can cause different behaviours for more complex event mappings.**

>NOTE: the fuel_main_valve gui button and hardware channel have no relation at the
beginning. Only an entry in the EventMapping links them together.

## Configuration

### config.json

In the config.json file all variables are defined that are important for the
initialization process of the llserver. A complete example config file with
all possible entries can be viewed in the [config_ecui](https://github.com/SpaceTeam/config_ecui) repo.

### mapping.json

The mapping.json file consists of four different parts. Each one must be at least
declared as an empty object, i.e.

```
{
    "CANMapping": {},
    "DefaultEventMapping": {},
    "EventMapping": {},
    "GUIMapping": {}
}
```
#### CANMapping

```
"CANMapping": {
    "7": {
            "0": {
                "offset": 0,
                "slope": 0.00010071,
                "stringID": "pmu_5V_voltage"
            },
            "1": {
                "offset": 0,
                "slope": 0.00010071,
                "stringID": "pmu_5V_high_load_voltage"
            },
            "2": {
                "offset": 0,
                "slope": 0.00033234,
                "stringID": "pmu_12V_voltage"
            }
            "stringID": "pmu"
        }
    },
```
Since in a CAN Network multiple **Nodes** comunicate with each other using IDs, 
we want to assign readable names to each node and each channel, so we
can work with them more easily.
A node entry starts with its unique identifier as a key and contains an object,
with arbitrary many channel entries. Each channel has a `<channel_name>:sensor` state that
can be scaled with the two entries `offset` and `slope`. The readable name
is defined with `stringID`.
A specification of the channel type is not needed since this information is loaded
when the nodes get initialized.


#### DefaultEventMapping

The default event mapping can be used to define generic behaviour for gui elements
acting on hardware channels.
```
"DefaultEventMapping": {
    "DigitalOut": [
        {
            "command": "DigitalOut:SetState",
            "parameters": [
                "DigitalOut"
            ]
        }
    ],
    "Servo": [
        {
            "command": "Servo:SetTargetPosition",
            "parameters": [
                "Servo"
            ]
        }
    ]
}
```
The default behaviour for a gui element can be overwritten by an entry
in the event mapping. The key of an action may be an hardware channel type
which defines an action for each hardware channel of this type. The 
channel type name can be also used to describe the command and parameters.
This gets then replaced with the actual channel name that is currently processed.

#### EventMapping
```
"EventMapping": {
    "gui:Flash:Clear": [
        {
            "command": "ecu:RequestFlashClear",
            "parameters": []
        },
        {
            "command": "pmu:RequestFlashClear",
            "parameters": []
        },
        {
            "command": "rcu:RequestFlashClear",
            "parameters": []
        }
    ]
}
```
An entry in the event mapping prevents default behaviours defined in the
DefaultEventMapping from being triggered. For further information about
events go to the [Events](#events) section.
#### GUIMapping
```
"GUIMapping": [
    {
        "label": "Fuel Tank Pressure",
        "state": "fuel_tank_pressure:sensor"
    },
    {
        "label": "Ox Tank Pressure",
        "state": "ox_tank_pressure:sensor"
    },
    {
        "label": "Supercharge Valve",
        "state": "supercharge_valve:sensor"
    }
]
```

The GUI Mapping is a config option to tell the user interface, how a gui element
with a certain state shall be named in an even more human readable way as state names.
This information only gets transmitted to the user interface and has no implications
on the llserver.

## Autonomous Control Test Sequence

To be able to test a liquid engine it is normally required to execute multiple commands
in a very short amount of time (pressurization, ignition, engine ramp-up, etc.).

For this purpose a sequence manager has been developed for automatic test sequences.

Test Sequences are defined inside the `$ECUI_CONFIG_PATH/sequences/` folder.
Each sequence uses the JSON format and consists of two main objects:
- `globals` - general definitions used for the sequence processing
- `data` - the actual sequence

### `globals` section
Inside the `globals` section following entries are needed

| key               | type   | value                                                                                                                      |
| ----------------- | ------ | -------------------------------------------------------------------------------------------------------------------------- |
| `"startTime"`     | float  | relative start of the squence (may also be negative)                                                                       |
| `"endTime"`       | float  | relative end of the sequence                                                                                               |
| `"interpolation"` | object | object with command name as key and either `"none"`(step function behaviour) or `"linear"`(linear interpolation) as values |
| `"interval"`      | float  | time precision of each iteration step                                                                                      |
| `"ranges"`        | array  | list of state names used for range checking                                                                                |

The interpolation entry specifies the behaviour between datapoints of the sequence.
The range entry is used to specify each state used for range checking. This means that at each datapoint a range can be
set in which the specified sensor must reside. If the sensor value gets out of range and **autoabort** is active inside the config.json
the sequence gets aborted instantly.

### `data` section

The data section includes a list of **command group**. These can be
used to group multiple datapoints relatively to one timestamp in order
to be moved more easily along the time axis if for example the burntime changes.

Each group command includes
 | key           | type            | value                                                                                                                         |
 | ------------- | --------------- | ----------------------------------------------------------------------------------------------------------------------------- |
 | `"timestamp"` | string or float | if timestamp is string only `"START"` or `"END"` are allowed which get replace by `"startTime"` and `"endTime"` respectively. |
 | `"name"`      | string          | name of the command group                                                                                                     |
 | `"desc"`      | string          | description                                                                                                                   |
 | `"actions"`   | array           | list of datapoints with relative timestamp to command group                                                                   |
 |               |


Each datapoint may include
 | key                    | type            | value                                                                                                            |
 | ---------------------- | --------------- | ---------------------------------------------------------------------------------------------------------------- |
 | `"timestamp"`          | float           | timestamp relative to command group (here **only floats are valid!**)                                            |
 | `"<command_name>"`     | array of floats | parameters for the specific command                                                                              |
 | `"sensorNominalRange"` | object          | states as keys and array of two elements with range as values (must be defined in the global section `"ranges"`) |
 

The first datapoint of the first command group **HAS TO** include all commands used in the sequence for proper initialization!
Each number in actions except the timestamp is double on the LLServer.

> Note: The keywords "START" or "END" are only allowed in the Group Commands (objects inside data array).
>
Example:
```
{
	"globals":
	{
		"endTime": 15,
		"interpolation":
		{
			"fuel_main_valve:SetTargetPosition": "linear",
			"ox_main_valve:SetTargetPosition": "linear",
			"igniter:SetState": "none"
		},
		"interval": 0.01,
		"ranges":
		[
			"chamberPressure:sensor"
		],
		"startTime": -10
	},
	"data":
	[
		{
			"timestamp": "START",
			"name": "start",
			"desc": "set all to initial state",
			"actions":
			[
				{
					"timestamp": 0.0,
					"fuel_main_valve:SetTargetPosition": [0],
                    "ox_main_valve:SetTargetPosition": [0],
					"igniter:SetState": [0], 
					"sensorsNominalRange":
					{
						"chamberPressure:sensor": [-5, 20]
					}
				}
			]
		},
		{
			"timestamp": -3.0,
			"name": "ignition",
			"desc": "lets light this candle",
			"actions":
			[
				{
					"timestamp": 0,
					"igniter:SetState": [100]
				}
			]
		},
		{
			"timestamp": 0.0,
			"name": "engine startup",
			"desc": "ramp up engine",
			"actions":
			[
				{
					"timestamp": 0,
					"fuel_main_valve:SetTargetPosition": [0],
					"ox_main_valve:SetTargetPosition": [0]
				},
				{
					"timestamp": 0.5,
					"ox_main_valve:SetTargetPosition": [15000],
					"fuel_main_valve:SetTargetPosition": [15000]
				},
				{
					"timestamp": 1.2,
					"fuel_main_valve:SetTargetPosition": [25000],
					"ox_main_valve:SetTargetPosition": [25000]
				},
				{
					"timestamp": 1.5,
					"igniter:SetState": [0]
				},
				{
					"timestamp": 1.7,
					"fuel_main_valve:SetTargetPosition": [100],
					"ox_main_valve:SetTargetPosition": [100]	
				}
			]
		},
		{
			"timestamp": 10.0,
			"name": "shutdown",
			"desc": "engine shutdown",
			"actions":
			[
				{
					"timestamp": 0,
					"fuel_main_valve:SetTargetPosition": [100],
					"ox_main_valve:SetTargetPosition": [100]
				},
                {
					"timestamp": 0.7,
					"fuel_main_valve:SetTargetPosition": [0],
					"ox_main_valve:SetTargetPosition": [0]
				}
			]
		},
		{
			"timestamp": "END",
			"name": "end",
			"desc": "the end",
			"actions":
			[
				{
					"timestamp": 0.0,
					"fuel_main_valve:SetTargetPosition": [0],
					"ox_main_valve:SetTargetPosition": [0]
				}
			]
		}
	]
}
```

### Abort Sequence Format

Another feature is the ability to define a **"safe state"** to be
executed in order the test control sequence fails or gets aborted.

Test Abort Sequence is defined inside the `$ECUI_CONFIG_PATH/sequences/abort_sequences/` folder. Currently only one abort sequence is supported and
must be named `AbortSequence.json`.

For now exactly one object with all commands to be executed.
The `"endTime"` key in `"globals"` is used to describe for how long the logging should continue
in case of an abort.

	{  
	    "globals": {
            "endTime": 3.2                              //double
        },
	    "actions" : {  
	        "fuel_main_valve:SetTargetPosition": [0],  	//array[double]
		    "igniter:SetState": [0],  				    //array[double]
		    "ox_main_valve:SetTargetPosition": [0]      //array[double]
	    }  
	}

## TCP Socket Message Types

In order to communicate with the webserver, a tcp socket connection is 
established. **It is mandatory to send the llserver specific messages
to initate state transmission and start test control sequences.**

For the whole API documentation refer to [Webserver](https://github.com/SpaceTeam/web_ecui_houbolt) 

## UDP Socket Endpoint for LoRa

This protocol is based on our [CAN Protocol](#can-protocol).

This implementation uses LoRa as an unidirectional method for receiving
data from the rocket during flight. For receiving LoRa messages a custom
LoRa shield is used on a raspberry pi. (Repo coming soon)
A UDP socket connection is used for data transmission between llserver and 
raspberry pi.

After the internal control
of the rocket takes over and no further commands are sent to the rocket
the only message type that the rocket sends automatically and periodically
is the `DataMsg_t` of each Generic Channel. This means we can strip the LoRa
message down to only data messages and can even combine them into one large
message. This is done on our RCU (Radio Control Unit) which listens to the
CAN FD bus for any data messages and updates a buffer which is split into
as many regions as generic channels exists inside the rocket (number of CAN
nodes). 

Since the data message length is highly variable and considered to be 
specified within the channel_mask, the header alone takes up an unnessesary 
large amount of bytes that can be considered "constant" during 
flight. Therefore the CAN header + data message header gets removed 
for the LoRa messages
and is statically entered inside the [config.json](#configjson).

There is only one additional byte for each generic channel which indicates if the 
section contains valid data. **THIS BYTE IS NOT INCLUDED IN THE CONFIG canMsgSize ENTRY!**

### LoRa Config

The `LORA` section in the [config.json](#configjson) includes

```
"LORA": {
    "ip": "192.168.1.7",
    "port": 5001,
    "nodeIDsRef": [6, 8],
    "nodeIDs": [16, 18],
    "canMsgSizes": [54, 39]
}
```

The `ip` and `port` describe the udp endpoint to the raspberry pi.

The `nodeIDsRef` array defines the corresponding node ids on the CAN FD bus. This
is needed since the LoRa messages are injected in the CAN Manager as normal CAN messages
and therefore need to load the correct CAN header before the message is injected. This
means that the hardware **MUST** be connected over the CAN FD bus for the initialization 
of the llserver in order to initialize the nodes correctly. Otherwise the LoRa messages cannot
be decoded and are lost!

The `nodeIDs` array contains the node ids for the LoRa channels (in this case CAN FD bus node ids
prepended with 1)

The `canMsgSizes` array contains the message size of each can data message (**HEADER BYTE EXCLUDED**)


>NOTE: that depending on the total message length, the LoRa driver on
the raspberrypi must be configured correctly as well. See (LoRa doc coming
soon...)


## Troubleshooting

- Every Sequence needs to define each device at the "START" timestamp
- **Make sure every "sensorsNominalRange" object in the sequence contains ALL sensors, with a range
specified**


- if the llserver fails to connect to the web server or carshes instantly try
to check if the correct ip addresses are used in the config file with
`sudo docker network inspect bridge`

- if either or both web and llserver refuse to start, try and check the docker environment variable for the correct config path

- if the llserver crashes instantly check if influx is correctly installed