//
// Created by Markus on 2019-09-29.
//

#ifndef TXV_ECUI_LLSERVER_SERIAL_H
#define TXV_ECUI_LLSERVER_SERIAL_H

#include <string>
#include <functional>

#include "common.h"

struct HCP_MSG
{
	uint8 optcode;
	size_t payloadSize;
	uint8 *payload = nullptr;
};

class Serial {

private:
	std::string _uartDevice;
	int _baudRate;
	int _uartFilestream;
    //unsigned int _msgLength = 0;
    //int _remainingBytes = 0;
    //char _buffer[255];
    //char* _currBufferPtr = &_buffer[0];

public:

    Serial(std::string device, int baudRate);
    ~Serial();
    HCP_MSG* ReadSync();
    void ReadAsync(std::function<void(HCP_MSG)> callback);
    void Write(HCP_MSG message);

    std::string GetDevice();
    int GetBaud();

};

#endif //TXV_ECUI_LLSERVER_SERIAL_H
