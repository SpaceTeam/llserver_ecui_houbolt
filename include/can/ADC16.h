//
// Created by Markus on 31.03.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_ADC16_H
#define LLSERVER_ECUI_HOUBOLT_ADC16_H

#include "can/Channel.h"

class ADC16 : public Channel
{
	public:
		ADC16(uint8_t id);
		~ADC16();
};

#endif //LLSERVER_ECUI_HOUBOLT_ADC16_H
