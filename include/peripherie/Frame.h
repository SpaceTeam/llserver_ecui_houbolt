#ifndef PERIPHERIE_FRAME_H
#define PERIPHERIE_FRAME_H

enum protocol_type {
	CAN,
};

struct peripherie_frame {
	int64_t id;
	enum protocol_type protocol;

	// NOTE(Lukas Karafiat): preliminary size of can protocol can be increased in the future
	size_t payload_size;
	uint8_t payload[8];
};

#endif /* PERIPHERIE_FRAME_H */
