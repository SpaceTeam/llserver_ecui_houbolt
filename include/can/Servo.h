//
// Created by Markus on 31.03.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_SERVO_H
#define LLSERVER_ECUI_HOUBOLT_SERVO_H

#include "can/Channel.h"

class Servo : public Channel
{
	public:
		Servo(uint8_t id);
		~Servo();
};

#endif //LLSERVER_ECUI_HOUBOLT_SERVO_H
