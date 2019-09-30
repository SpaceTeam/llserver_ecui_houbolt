//
// Created by Markus on 2019-09-27.
//

#ifndef TXV_ECUI_LLSERVER_CONFIG_H
#define TXV_ECUI_LLSERVER_CONFIG_H

#define SEQUENCE_FILE_PATH "../TXV_ECUI_WEB/sequence/Sequence.json"
#define ABORT_EQUENCE_FILE_PATH "../TXV_ECUI_WEB/sequence/AbortSequence.json"

#define SOCKET_MSG_SIZE 65536
#define PORT 5555

#define TIMER_SYNC_INTERVAL 5000000 //us

#define MAPPING_FILE_PATH "mapping/mapping.json"

#define HCP_DEVICE "/dev/serial0"
#define HCP_BAUD_RATE 115200

#define SERVO_COUNT 6
#define SERVO_MIN_ONTIME 0
#define SERVO_MAX_ONTIME 40000

#define DIGITAL_COUNT 16

#define ANALOG_COUNT 16

//voltages in mV
#define BATTERY_FULL 12600
#define BATTERY_EMPTY_THRESHOLD 9100 //battery voltage below which the power gets switched off
#define BATTERY_LOW_THRESHOLD 9900 //battery voltage below which low battery indication occurs
#define BATTERY_VOLTAGE_HYSTERESIS 100

#endif //TXV_ECUI_LLSERVER_CONFIG_H
