#include <unistd.h>			//Used for UART
#include <fcntl.h>			//Used for UART
#include <termios.h>		//Used for UART
#include <iostream>
#include <cstring>

#include "common.h"
#include "HcpCommands.h"

#include "Serial.h"

#define MSG_SIZE 2048

using namespace std;

Serial::Serial(string device, int baudRate)
{
	_uartDevice = device;
	_baudRate = baudRate;

	_uartFilestream = -1;

	_uartFilestream = open(device.c_str(), O_RDWR | O_NOCTTY);	
    close(_uartFilestream);
    _uartFilestream = open(device.c_str(), O_RDWR | O_NOCTTY);  
    if (_uartFilestream == -1)
    {
        //ERROR - CAN'T OPEN SERIAL PORT
        Debug::error("Error - Unable to open UART.  Ensure it is not in use by another application\n");
        return;
    }
    else
    {
        Debug::print("Filestream created sucessfully");
    }

    struct termios options;
    tcgetattr(_uartFilestream, &options);
    options.c_cflag = CS8 | CLOCAL | CREAD;		//<Set baud rate

    switch(baudRate)
    {
    	//NOTE: cases commented out are not supported on arduino
    	/*case 0:
    		options.c_cflag |= B0;
    		break;

    	case 50:
    		options.c_cflag |= B50;
    		break;

    	case 75:
    		options.c_cflag |= B75;
    		break;

    	case 110:
    		options.c_cflag |= B110;
    		break;

    	case 134:
    		options.c_cflag |= B134;
    		break;

    	case 150:
    		options.c_cflag |= B150;
    		break;

    	case 200:
    		options.c_cflag |= B200;
    		break;
		*/
    	case 300:
    		options.c_cflag |= B300;
    		break;

    	case 600:
    		options.c_cflag |= B600;
    		break;

    	case 1200:
    		options.c_cflag |= B1200;
    		break;

    	case 1800:
    		options.c_cflag |= B1800;
    		break;

    	case 2400:
    		options.c_cflag |= B2400;
    		break;

    	case 4800:
    		options.c_cflag |= B4800;
    		break;

    	case 9600:
    		options.c_cflag |= B9600;
    		break;

    	//NOTE: 14400 not supported by termios
    	case 19200:
    		options.c_cflag |= B19200;
    		break;
    	//NOTE: 28800 not supported by termios
    	case 38400:
    		options.c_cflag |= B38400;
    		break;

    	case 57600:
    		options.c_cflag |= B57600;
    		break;

    	case 115200:
    		options.c_cflag |= B115200;
    		break;

    	case 230400:
    		options.c_cflag |= B230400;
    		break;

    	default:
    		cerr << "no valid baudrate found --- falling back to 9600" << endl;
    		options.c_cflag |= B9600;
    }

    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(_uartFilestream, TCIFLUSH);
    tcsetattr(_uartFilestream, TCSANOW, &options);
}

Serial::~Serial()
{
	close(_uartFilestream);
}

HCP_MSG* Serial::ReadSync()
{
    //----- CHECK FOR ANY RX BYTES -----
    uint8 rxBuffer[MSG_SIZE] = {0};
    //msg.msg = rxBuffer;
    bool finished = false;
    if (_uartFilestream != -1)
    {
        while(!finished)
        {
            int rxLength = read(_uartFilestream, (void *) rxBuffer,
                                1);        //Filestream, buffer to store in, number of bytes to read (MSG_SIZE)
            if (rxLength < 0)
            {
                //NOTE: if this occurs settings of serial com is broken --> non blocking
                cout << "optcode: no bytes recieved" << endl;
            }
            else if (rxLength == 0)
            {
                //No data waiting
//                cout << "optcode: no data...waiting" << endl;
            }
            else
            {
                HCP_MSG* msg = new HCP_MSG;

                msg->optcode = rxBuffer[0];


                int msgLength = hcp_cmds[msg->optcode].payloadLength;
                cout << "Message Length: " << msgLength << endl;
                int remainingBytes = msgLength;


                msg->payloadSize = msgLength;
                msg->payload = new uint8[msg->payloadSize];

                while (remainingBytes > 0)
                {
                    rxLength = read(_uartFilestream, (void *) rxBuffer, remainingBytes);
//                    cout << rxLength << " bytes recieved" << endl;
                    if (rxLength < 0)
                    {
                        //NOTE: if this occurs settings of serial com is broken --> non blocking
                        cout << "payload: no bytes recieved" << endl;
                    }
                    else if (rxLength == 0)
                    {
                        //No data waiting
//                        cout << "payload: no data...waiting" << endl;
                    }
                    else
                    {
                        std::memcpy(&msg->payload[msgLength - remainingBytes],
                                    &rxBuffer[0],
                                    rxLength);
                        remainingBytes -= rxLength;
                    }
//                    sleep(1);
                }

                finished = true;
                return msg;

            }
        }

    //	sleep(1);
    }
    else
    {
        Debug::error("Device " + _uartDevice + " disconnected!");
    }
    return nullptr;
}

void Serial::ReadAsync(std::function<void(HCP_MSG)> callback)
{
    //----- CHECK FOR ANY RX BYTES -----
    uint8 rxBuffer[MSG_SIZE] = {0};
    //msg.msg = rxBuffer;
    bool finished = false;

    if (_uartFilestream != -1)
    {
        while(!finished)
        {
            int rxLength = read(_uartFilestream, (void *) rxBuffer,
                                1);        //Filestream, buffer to store in, number of bytes to read (MSG_SIZE)
            if (rxLength < 0)
            {
                //NOTE: if this occurs settings of serial com is broken --> non blocking
                cout << "optcode: no bytes recieved" << endl;
            }
            else if (rxLength == 0)
            {
                //No data waiting
//                cout << "optcode: no data...waiting" << endl;
            }
            else
            {
                HCP_MSG msg;

                msg.optcode = rxBuffer[0];


                int msgLength = hcp_cmds[msg.optcode].payloadLength;
                cout << "Message Length: " << msgLength << endl;
                int remainingBytes = msgLength;


                msg.payloadSize = msgLength;
                msg.payload = new uint8[msg.payloadSize];

                while (remainingBytes > 0)
                {
                    rxLength = read(_uartFilestream, (void *) rxBuffer, remainingBytes);
//                    cout << rxLength << " bytes recieved" << endl;
                    if (rxLength < 0)
                    {
                        //NOTE: if this occurs settings of serial com is broken --> non blocking
                        cout << "payload: no bytes recieved" << endl;
                    }
                    else if (rxLength == 0)
                    {
                        //No data waiting
//                        cout << "payload: no data...waiting" << endl;
                    }
                    else
                    {
                        std::memcpy(&msg.payload[msgLength - remainingBytes],
                                    &rxBuffer[0],
                                    rxLength);
                        remainingBytes -= rxLength;
                    }
//                    sleep(1);
                }

                finished = true;
                callback(msg);

            }
        }

//	sleep(1);
    }
    else
    {
        Debug::error("Device " + _uartDevice + " disconnected!");
    }
}

void Serial::Write(HCP_MSG message)
{
    uint8 buffer[message.payloadSize+1];
    buffer[0] = message.optcode;
    std::copy(&message.payload[0], &message.payload[message.payloadSize-1], &buffer[1]);
    if (_uartFilestream != -1)
    {
        int count = write(_uartFilestream, &buffer[0], message.payloadSize+1);        //Filestream, bytes to write, number of bytes to write
        if (count < 0)
        {
            printf("UART Write error\n");
        }
    }
    else
    {
        Debug::error("Device " + _uartDevice + " disconnected!");
    }
}

bool Serial::IsConnected()
{
    return _uartFilestream != -1;
}

string Serial::GetDevice()
{
    return this->_uartDevice;
}

int Serial::GetBaud()
{
    return this->_baudRate;
}
