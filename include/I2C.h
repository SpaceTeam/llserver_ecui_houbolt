//
// Created by Markus on 2019-10-13.
//

#ifndef TXV_ECUI_LLSERVER_I2C_H
#define TXV_ECUI_LLSERVER_I2C_H

#include "common.h"

class I2C
{

private:

    int devId;
	int baudRate;
	int i2cFile;

	bool connected = false;

	std::mutex i2cMtx;

public:

    I2C(int devId);

    ~I2C();

    uint8 ReadByte();
    bool WriteByte(uint8 byte);

};


#endif //TXV_ECUI_LLSERVER_I2C_H
