#ifndef STATE_H
#define STATE_H

#include <variant>

using state = std::variant<bool, int, double>;

enum peripherie_type {
	CAN_SOCKET,
};

struct actuator {
	char name[1024];
	state value;

	// transformation information
	enum peripherie_type peripherie_type;
};

struct sensor {
	char name[1024];
	state value;
};

#endif /* STATE_H */
