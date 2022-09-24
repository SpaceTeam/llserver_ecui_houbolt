#ifndef STATE_H
#define STATE_H

enum peripherie_type {
	CAN_SOCKET,
};

struct actuator {
	double value;

	int node_id;
	int channel_id;
	int value_id;
	int command_id;

	enum peripherie_type peripherie_type;
};

struct sensor {
	double value;

	int node_id;
	int channel_id;
	int value_id;
	int command_id;

	enum peripherie_type peripherie_type;
};

#endif /* STATE_H */
