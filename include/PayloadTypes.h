#ifndef PAYLOAD_TYPES_H
#define PAYLOAD_TYPES_H

struct get_payload {
	uint8_t variable_id;
} __attribute__ ((__packed__));

struct set_payload {
	uint8_t variable_id;
	uint32_t value;
} __attribute__ ((__packed__));

struct servo_move_payload {
	uint32_t position;
	uint32_t interval;
} __attribute__ ((__packed__));

struct rocket_state_payload {
	uint32_t state;
} __attribute__ ((__packed__));

#endif /* PAYLOAD_TYPES_H */
