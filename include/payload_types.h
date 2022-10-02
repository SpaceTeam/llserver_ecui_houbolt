#ifndef PAYLOAD_TYPES_H
#define PAYLOAD_TYPES_H

// enum class channel_status : uint32_t {
// 	noice,
// 	type_unknown,
// 	sensor_not_connected,
// 	wrong_readings,
// 	error,
// 	no_channel
// };

// enum class threshold_operation : uint32_t {
// 	equals,
// 	less_than,
// 	greater_than
// };

struct get_payload {
	uint8_t variable_id;
} __attribute__ ((__packed__));

struct set_payload {
	uint8_t variable_id;
	uint32_t value;
} __attribute__ ((__packed__));

// struct imu_payload {
// 	uint16_t x_accel;	// x-axis linear acceleration
// 	uint16_t y_accel;	// y-axis linear acceleration
// 	uint16_t z_accel;	// z-axis linear acceleration
// 	uint16_t x_alpha;	// x-axis angular acceleration
// 	uint16_t y_alpha;	// y-axis angular acceleration
// 	uint16_t z_alpha;	// z-axis angular acceleration
// } __attribute__ ((__packed__));

struct get_rocket_state_payload {
	uint32_t state;
} __attribute__ ((__packed__));

struct set_rocket_state_payload {
	uint32_t state;
	uint32_t status;
} __attribute__ ((__packed__));

struct servo_move_payload {
	uint32_t position;
	uint32_t interval;
} __attribute__ ((__packed__));

struct node_info_payload {
	uint32_t firmware_version;
	uint32_t channel_mask;
	uint8_t channel_type[32];
} __attribute__((__packed__));

// struct node_status_payload {
// 	uint32_t node_error_flags;
// 	uint32_t channel_error_mask;
// 	uint8_t channel_status[32];
// } __attribute__((__packed__));

// struct data_message {
// 	uint32_t channel_mask;
// 	uint8_t data[64 - sizeof(uint32_t)];
// } __attribute__((__packed__));

struct speaker_payload {
	uint16_t tone_frequency;	// in Hz
	uint16_t on_time;		// in ms
	uint16_t off_time;		// in ms
	uint8_t count;			// number of beeps
} __attribute__((__packed__));

// struct threshold_payload {
// 	uint8_t channel_id;
// 	uint8_t threshold_id;
// 	uint8_t enabled;		// (0, 1)
// 	uint8_t variable_id;
// 	uint32_t compare_id;
// 	int32_t threshold;
// 	channel_status result;
// 	uint8_t or_threshold_id;
// 	uint8_t and_threshold_id;
// } __attribute__((__packed__));

//struct flash_status_payload {
//	uint8_t status;
//} __attribute__((__packed__));

#endif /* PAYLOAD_TYPES_H */
