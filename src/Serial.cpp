#include <unistd.h>			//Used for UART
#include <fcntl.h>			//Used for UART
#include <termios.h>		//Used for UART
#include <iostream>
#include <cstring>

#include "common.h"
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
        cerr << "Error - Unable to open UART.  Ensure it is not in use by another application\n" << endl;
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

void Serial::Read(std::function<void(Hedgehog_Msg)> callback)
{
    //----- CHECK FOR ANY RX BYTES -----
    char rxBuffer[MSG_SIZE] = {0};
    //msg.msg = rxBuffer;

//    while(true)
//    {
        if (_uartFilestream != -1)
        {

                int msgLength;

                Hedgehog_Msg msg;

                    msgLength = read(_uartFilestream, (void *) rxBuffer, MSG_SIZE);
                    cout << msgLength << " bytes recieved" << endl;
                    if (msgLength < 0)
                    {
                        //NOTE: if this occurs settings of serial com is broken --> non blocking
                        cout << "no bytes recieved" << endl;
                    }
                    else if (msgLength == 0)
                    {
                        //No data waiting
                        cout << "no data...waiting" << endl;
                    }
                    else
                    {
			msg.size = msgLength;
			msg.msg = new char[msg.size];
                        std::memcpy(&msg.msg[0],
                                    &rxBuffer[0],
                                    msgLength);
                    }
                callback(msg);
                //cout << "end: -------------------" << endl;

                //Bytes received

            }
//	sleep(1);
//    }
    
}

void Serial::Write(string message)
{
	const char* msg = message.c_str();
    if (_uartFilestream != -1)
    {
        int count = write(_uartFilestream, &msg[0], message.size());		//Filestream, bytes to write, number of bytes to write
        if (count < 0)
        {
            printf("UART Write error\n");
        }
    }
    else
    {
    	cerr << "Device " << _uartDevice << " disconnected!" << endl;
    }
}

void Serial::Write(Hedgehog_Msg message)
{
    if (_uartFilestream != -1)
    {
        int count = write(_uartFilestream, &message.msg[0], message.size);        //Filestream, bytes to write, number of bytes to write
        if (count < 0)
        {
            printf("UART Write error\n");
        }
    }
    else
    {
        cerr << "Device " << _uartDevice << " disconnected!" << endl;
    }
}

string Serial::GetDevice()
{
    return this->_uartDevice;
}

int Serial::GetBaud()
{
    return this->_baudRate;
}
