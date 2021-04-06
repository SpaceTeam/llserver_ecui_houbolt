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
    std::string devName;
	int baudRate;
	int i2cFile;

	bool connected = false;

	std::mutex i2cMtx;

public:

    I2C(int devId, std::string devName);

    ~I2C();

    uint8_t Read8();
    uint16_t Read16();
    bool Write8(uint8_t byte);

    std::string GetName();

};


#endif //TXV_ECUI_LLSERVER_I2C_H
