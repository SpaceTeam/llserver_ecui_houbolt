//
// Created by Markus on 2019-09-29.
//

#ifndef TXV_ECUI_LLSERVER_HCPCOMMANDS_H
#define TXV_ECUI_LLSERVER_HCPCOMMANDS_H

#include "common.h"
//unused opcodes, if all 256 opcodes are used, then ACP_UNUSED should not be defined
#define HCP_UNUSED				0xFF

//commands from RPi
#define HCP_VERS_REQ			0x01
#define	HCP_EMERGENCY_ACTION    0x05
#define	HCP_EMERGENCY_REQ       0x06
#define HCP_IO_CONFIG        	0x10
#define HCP_ANALOG_REQ      	0x20
#define HCP_IMU_RATE_REQ		0x22
#define HCP_IMU_ACCEL_REQ		0x23
#define HCP_IMU_POSE_REQ		0x24
#define HCP_DIGITAL_REQ     	0x30
#define HCP_MOTOR           	0x40
#define HCP_MOTOR_POSITIONAL	0x41
#define HCP_MOTOR_SERVO			0x42
#define HCP_MOTOR_CONFIG_DC		0x48
#define HCP_MOTOR_CONFIG_ENC	0x49
#define HCP_MOTOR_CONFIG_STEP	0x4A
#define HCP_SERVO           	0x50
#define HCP_UART          		0x60
#define HCP_SPEAKER				0x70
#define HCP_ST_THRUST_REQ		0x93

//replies to RPi
#define HCP_VERS_REP			0x02
#define	HCP_EMERGENCY_REP       0x07
#define HCP_OK              	0x80
#define HCP_UNKNOWN_OPCODE 		0x81
#define HCP_UNSUPPORTED_OPCODE 	0x82
#define HCP_INVALID_PORT    	0x83
#define HCP_INVALID_CONFIG     	0x84
#define HCP_INVALID_MODE    	0x85
#define HCP_INVALID_FLAGS   	0x86
#define HCP_INVALID_VALUE   	0x87
#define HCP_FAIL_EMERG_ACT      0x88
#define HCP_ST_THRUST_REP		0x94
#define HCP_ANALOG_REP      	0xA1
#define HCP_IMU_RATE_REP      	0xA2
#define HCP_IMU_ACCEL_REP      	0xA3
#define HCP_IMU_POSE_REP      	0xA4
#define HCP_DIGITAL_REP     	0xB1

//updates to RPi
#define HCP_SHUTDOWN			0x03
#define HCP_EMERGENCY_STOP		0x04
#define HCP_MOTOR_DONE_UPDATE	0xC3
#define HCP_UART_UPDATE   		0xE1

//motor modes
#define HCP_MOTOR_MODE_POWER 	0x00
#define HCP_MOTOR_MODE_BRAKE 	0x01
#define HCP_MOTOR_MODE_VELOCITY 0x02

//motor_positional done_modes
#define HCP_MOTOR_POS_DONE_MODE_OFF 			0x00
#define HCP_MOTOR_POS_DONE_MODE_BRAKE 			0x01
#define HCP_MOTOR_POS_DONE_MODE_ACTIVE_BRAKE	0x02

//special analog/digital ports
#define HCP_ANALOG_SUPPLY_PORT 0x80
#define HCP_DIGITAL_LED0_PORT 	0x90
#define HCP_DIGITAL_LED1_PORT 	0x91

#define HCP_VPL_FLAG 0x01

typedef struct
{
	uint8 opcode; //the command's opcode
	uint8 flags; //flags regarding the command
	size_t payloadLength; //the command's payload length, or 0 if variable

} hcp_cmd_t;

//array of commands, indexed by the opcode
extern hcp_cmd_t hcp_cmds[256];


#endif //TXV_ECUI_LLSERVER_HHGCOMMANDS_H
