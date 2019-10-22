//
// Created by Markus on 2019-10-13.
//

#include "I2C.h"

#include <fcntl.h>

using namespace std;

#ifdef __linux__

#include <wiringPiI2C.h>

I2C::I2C(int devId, std::string devName)
{
    cout << "WiringPi detected" << endl;
    this->devId = devId;
    this->devName = devName;
    this->i2cFile = wiringPiI2CSetup(devId); //0x20

    if (this->i2cFile < 0)
    {
        cout << "Unable to connect i2c: no device found" << endl;
    }
    else
    {
        this->connected = true;
    }
}

I2C::~I2C()
{
    if (this->connected)
    {
        close(this->i2cFile);
    }
}

uint8 I2C::Read8()
{
    std::lock_guard<std::mutex> lock(i2cMtx);

    return wiringPiI2CRead(this->i2cFile);
}

uint16 I2C::Read16()
{
    std::lock_guard<std::mutex> lock(i2cMtx);

    return wiringPiI2CReadReg16(this->i2cFile, 0);
}

bool I2C::Write8(uint8 byte)
{
    std::lock_guard<std::mutex> lock(i2cMtx);

    bool success = true;

    //cout << "write 0x" << std::hex << (int)byte << " to i2c" << endl;
    if (wiringPiI2CWrite(this->i2cFile, byte) < 0)
    {
        success = false;
    }

    return success;
}


#else

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

#endif

std::string I2C::GetName()
{
    return this->devName;
}
