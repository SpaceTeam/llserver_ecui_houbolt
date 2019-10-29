//
// Created by Markus on 2019-09-29.
//

#include "HcpCommands.h"


hcp_cmd_t hcp_cmds[256] = {
	{HCP_UNUSED},																		//   0 0x00
	{HCP_VERS_REQ, 0, 0},											//   1 0x01
	{HCP_VERS_REP, 0, 14},														//   2 0x02
	{HCP_SHUTDOWN, 0, 0},															//   3 0x03
	{HCP_EMERGENCY_STOP, 0, 0},													//   4 0x04
	{HCP_EMERGENCY_ACTION, 0, 1},								//   5 0x05
	{HCP_EMERGENCY_REQ, 0, 0},									//   6 0x06
	{HCP_EMERGENCY_REP, 0, 1},													//   7 0x07
	{HCP_UNUSED},																		//   8
	{HCP_UNUSED},																		//   9
	{HCP_UNUSED},																		//  10
	{HCP_UNUSED},																		//  11
	{HCP_UNUSED},																		//  12
	{HCP_UNUSED},																		//  13
	{HCP_UNUSED},																		//  14
	{HCP_UNUSED},																		//  15
	{HCP_IO_CONFIG, 0, 2},										//  16 0x10
	{HCP_UNUSED},																		//  17
	{HCP_UNUSED},																		//  18
	{HCP_UNUSED},																		//  19
	{HCP_UNUSED},																		//  20
	{HCP_UNUSED},																		//  21
	{HCP_UNUSED},																		//  22
	{HCP_UNUSED},																		//  23
	{HCP_UNUSED},																		//  24
	{HCP_UNUSED},																		//  25
	{HCP_UNUSED},																		//  26
	{HCP_UNUSED},																		//  27
	{HCP_UNUSED},																		//  28
	{HCP_UNUSED},																		//  29
	{HCP_UNUSED},																		//  30
	{HCP_UNUSED},																		//  31
	{HCP_ANALOG_REQ, 0, 1},										//  32 0x20
	{HCP_UNUSED},																		//  33
	{HCP_IMU_RATE_REQ, 0, 0},											//  34 0x22
	{HCP_IMU_ACCEL_REQ, 0, 0},											//  35 0x23
	{HCP_IMU_POSE_REQ, 0, 0},											//  36 0x24
	{HCP_UNUSED},																		//  37
	{HCP_UNUSED},																		//  38
	{HCP_UNUSED},																		//  39
	{HCP_UNUSED},																		//  40
	{HCP_UNUSED},																		//  41
	{HCP_UNUSED},																		//  42
	{HCP_UNUSED},																		//  43
	{HCP_UNUSED},																		//  44
	{HCP_UNUSED},																		//  45
	{HCP_UNUSED},																		//  46
	{HCP_UNUSED},																		//  47
	{HCP_DIGITAL_REQ, 0, 1},									//  48 0x30
	{HCP_UNUSED},																		//  49
	{HCP_UNUSED},																		//  50
	{HCP_UNUSED},																		//  51
	{HCP_UNUSED},																		//  52
	{HCP_UNUSED},																		//  53
	{HCP_UNUSED},																		//  54
	{HCP_UNUSED},																		//  55
	{HCP_UNUSED},																		//  56
	{HCP_UNUSED},																		//  57
	{HCP_UNUSED},																		//  58
	{HCP_UNUSED},																		//  59
	{HCP_UNUSED},																		//  60
	{HCP_UNUSED},																		//  61
	{HCP_UNUSED},																		//  62
	{HCP_UNUSED},																		//  63
	{HCP_MOTOR, 0, 4},												//  64 0x40
	{HCP_MOTOR_POSITIONAL, 0, 9},									//  65 0x41
	{HCP_MOTOR_SERVO, 0, 8},											//  66 0x42
	{HCP_UNUSED},																		//  67
	{HCP_UNUSED},																		//  68
	{HCP_UNUSED},																		//  69
	{HCP_UNUSED},																		//  70
	{HCP_UNUSED},																		//  71
	{HCP_MOTOR_CONFIG_DC, 0, 1},										//  72 0x48
	{HCP_MOTOR_CONFIG_ENC, 0, 3},									//  73 0x49
	{HCP_MOTOR_CONFIG_STEP, 0, 1},									//  74 0x4A
	{HCP_UNUSED},																		//  75
	{HCP_UNUSED},																		//  76
	{HCP_UNUSED},																		//  77
	{HCP_UNUSED},																		//  78
	{HCP_UNUSED},																		//  79
	{HCP_SERVO, 0, 3},												//  80 0x50
	{HCP_UNUSED},																		//  81
	{HCP_UNUSED},																		//  82
	{HCP_UNUSED},																		//  83
	{HCP_UNUSED},																		//  84
	{HCP_UNUSED},																		//  85
	{HCP_UNUSED},																		//  86
	{HCP_UNUSED},																		//  87
	{HCP_UNUSED},																		//  88
	{HCP_UNUSED},																		//  89
	{HCP_UNUSED},																		//  90
	{HCP_UNUSED},																		//  91
	{HCP_UNUSED},																		//  92
	{HCP_UNUSED},																		//  93
	{HCP_UNUSED},																		//  94
	{HCP_UNUSED},																		//  95
	{HCP_UART, HCP_VPL_FLAG, 0},										//  96 0x60
	{HCP_UNUSED},																		//  97
	{HCP_UNUSED},																		//  98
	{HCP_UNUSED},																		//  99
	{HCP_UNUSED},																		// 100
	{HCP_UNUSED},																		// 101
	{HCP_UNUSED},																		// 102
	{HCP_UNUSED},																		// 103
	{HCP_UNUSED},																		// 104
	{HCP_UNUSED},																		// 105
	{HCP_UNUSED},																		// 106
	{HCP_UNUSED},																		// 107
	{HCP_UNUSED},																		// 108
	{HCP_UNUSED},																		// 109
	{HCP_UNUSED},																		// 110
	{HCP_UNUSED},																		// 111
	{HCP_SPEAKER, 0, 2},											// 112 0x70
	{HCP_UNUSED},																		// 113
	{HCP_UNUSED},																		// 114
	{HCP_UNUSED},																		// 115
	{HCP_UNUSED},																		// 116
	{HCP_UNUSED},																		// 117
	{HCP_UNUSED},																		// 118
	{HCP_UNUSED},																		// 119
	{HCP_UNUSED},																		// 120
	{HCP_UNUSED},																		// 121
	{HCP_UNUSED},																		// 122
	{HCP_UNUSED},																		// 123
	{HCP_UNUSED},																		// 124
	{HCP_UNUSED},																		// 125
	{HCP_UNUSED},																		// 126
	{HCP_UNUSED},																		// 127
	{HCP_OK, 0, 0},																// 128 0x80
	{HCP_UNKNOWN_OPCODE, 0, 0},													// 129 0x81
	{HCP_UNSUPPORTED_OPCODE, 0, 0},												// 130 0x82
	{HCP_INVALID_PORT, 0, 0},														// 131 0x83
	{HCP_INVALID_CONFIG, 0, 0},													// 132 0x84
	{HCP_INVALID_MODE, 0, 0},														// 133 0x85
	{HCP_INVALID_FLAGS, 0, 0},													// 134 0x86
	{HCP_INVALID_VALUE, 0, 0},													// 135 0x87
	{HCP_FAIL_EMERG_ACT, 0, 0},													// 136 0x88
	{HCP_UNUSED},																		// 137
	{HCP_UNUSED},																		// 138
	{HCP_UNUSED},																		// 139
	{HCP_UNUSED},																		// 140
	{HCP_UNUSED},																		// 141
	{HCP_UNUSED},																		// 142
	{HCP_UNUSED},																		// 143
	{HCP_UNUSED},																		// 144
	{HCP_UNUSED},																		// 145
	{HCP_UNUSED},																		// 146
	{HCP_ST_THRUST_REQ, 0, 0},											// 147 0x93
	{HCP_ST_THRUST_REP, 0, 9},													// 148 0x94
	{HCP_UNUSED},																		// 149
	{HCP_UNUSED},																		// 150
	{HCP_UNUSED},																		// 151
	{HCP_UNUSED},																		// 152
	{HCP_UNUSED},																		// 153
	{HCP_UNUSED},																		// 154
	{HCP_UNUSED},																		// 155
	{HCP_UNUSED},																		// 156
	{HCP_UNUSED},																		// 157
	{HCP_UNUSED},																		// 158
	{HCP_UNUSED},																		// 159
	{HCP_UNUSED},																		// 160
	{HCP_ANALOG_REP, 0, 3},														// 161 0xA1
	{HCP_IMU_RATE_REP, 0, 6},														// 162 0xA2
	{HCP_IMU_ACCEL_REP, 0, 6},													// 163 0xA3
	{HCP_IMU_POSE_REP, 0, 6},														// 164 0xA4
	{HCP_UNUSED},																		// 165
	{HCP_UNUSED},																		// 166
	{HCP_UNUSED},																		// 167
	{HCP_UNUSED},																		// 168
	{HCP_UNUSED},																		// 169
	{HCP_UNUSED},																		// 170
	{HCP_UNUSED},																		// 171
	{HCP_UNUSED},																		// 172
	{HCP_UNUSED},																		// 173
	{HCP_UNUSED},																		// 174
	{HCP_UNUSED},																		// 175
	{HCP_UNUSED},																		// 176
	{HCP_DIGITAL_REP, 0, 2},														// 177 0xB1
	{HCP_UNUSED},																		// 178
	{HCP_UNUSED},																		// 179
	{HCP_UNUSED},																		// 180
	{HCP_UNUSED},																		// 181
	{HCP_UNUSED},																		// 182
	{HCP_UNUSED},																		// 183
	{HCP_UNUSED},																		// 184
	{HCP_UNUSED},																		// 185
	{HCP_UNUSED},																		// 186
	{HCP_UNUSED},																		// 187
	{HCP_UNUSED},																		// 188
	{HCP_UNUSED},																		// 189
	{HCP_UNUSED},																		// 190
	{HCP_UNUSED},																		// 191
	{HCP_UNUSED},																		// 192
	{HCP_UNUSED},																		// 193
	{HCP_UNUSED},																		// 194
	{HCP_MOTOR_DONE_UPDATE, 0, 1},												// 195 0xC3
	{HCP_UNUSED},																		// 196
	{HCP_UNUSED},																		// 197
	{HCP_UNUSED},																		// 198
	{HCP_UNUSED},																		// 199
	{HCP_UNUSED},																		// 200
	{HCP_UNUSED},																		// 201
	{HCP_UNUSED},																		// 202
	{HCP_UNUSED},																		// 203
	{HCP_UNUSED},																		// 204
	{HCP_UNUSED},																		// 205
	{HCP_UNUSED},																		// 206
	{HCP_UNUSED},																		// 207
	{HCP_UNUSED},																		// 208
	{HCP_UNUSED},																		// 209
	{HCP_UNUSED},																		// 210
	{HCP_UNUSED},																		// 211
	{HCP_UNUSED},																		// 212
	{HCP_UNUSED},																		// 213
	{HCP_UNUSED},																		// 214
	{HCP_UNUSED},																		// 215
	{HCP_UNUSED},																		// 216
	{HCP_UNUSED},																		// 217
	{HCP_UNUSED},																		// 218
	{HCP_UNUSED},																		// 219
	{HCP_UNUSED},																		// 220
	{HCP_UNUSED},																		// 221
	{HCP_UNUSED},																		// 222
	{HCP_UNUSED},																		// 223
	{HCP_UNUSED},																		// 224
	{HCP_UART_UPDATE, HCP_VPL_FLAG, 0},											// 225 0xE1
	{HCP_UNUSED},																		// 226
	{HCP_UNUSED},																		// 227
	{HCP_UNUSED},																		// 228
	{HCP_UNUSED},																		// 229
	{HCP_UNUSED},																		// 230
	{HCP_UNUSED},																		// 231
	{HCP_UNUSED},																		// 232
	{HCP_UNUSED},																		// 233
	{HCP_UNUSED},																		// 234
	{HCP_UNUSED},																		// 235
	{HCP_UNUSED},																		// 236
	{HCP_UNUSED},																		// 237
	{HCP_UNUSED},																		// 238
	{HCP_UNUSED},																		// 239
	{HCP_UNUSED},																		// 240
	{HCP_UNUSED},																		// 241
	{HCP_UNUSED},																		// 242
	{HCP_UNUSED},																		// 243
	{HCP_UNUSED},																		// 244
	{HCP_UNUSED},																		// 245
	{HCP_UNUSED},																		// 246
	{HCP_UNUSED},																		// 247
	{HCP_UNUSED},																		// 248
	{HCP_UNUSED},																		// 249
	{HCP_UNUSED},																		// 250
	{HCP_UNUSED},																		// 251
	{HCP_UNUSED},																		// 252
	{HCP_UNUSED},																		// 253
	{HCP_UNUSED},																		// 254
	{HCP_UNUSED},																		// 255
};