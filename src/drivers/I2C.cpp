//
// Created by Markus on 2019-10-13.
//

#include "drivers/I2C.h"

#include <fcntl.h>

using namespace std;

I2C::I2C(int devId, std::string devName)
{
    cout << "Not on LINUX ... simulate connection" << endl;
    this->connected = true;
    this->devId = devId;
    this->devName = devName;
}

I2C::~I2C()
{

}

uint8 I2C::Read8()
{
    std::lock_guard<std::mutex> lock(i2cMtx);

    return rand() % 255;
}

uint16 I2C::Read16()
{
    std::lock_guard<std::mutex> lock(i2cMtx);

    return rand() % 0xFFFF;
}

bool I2C::Write8(uint8 byte)
{
    std::lock_guard<std::mutex> lock(i2cMtx);

    bool success = true;

    //cout << "write 0x" << std::hex << (int)byte << " to i2c" << endl;

    return success;
}

std::string I2C::GetName()
{
    return this->devName;
}
