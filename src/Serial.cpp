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

	Debug::print("Opening Serial Connection...");
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
#ifdef B14400
        case 14400:
            options.c_cflag |= B14400;
            break;
#endif
    	case 19200:
    		options.c_cflag |= B19200;
    		break;
#ifdef B28800
        case 28800:
            options.c_cflag |= B28800;
            break;
#endif
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
#ifdef B460800
        case 460800:
            options.c_cflag |= B460800;
            break;
#endif
#ifdef B921600
        case 921600:
            options.c_cflag |= B921600;
            break;
#endif
    	default:
		    Debug::error("no valid baudrate found --- falling back to 9600");
    		options.c_cflag |= B9600;
    		//options.c_cflag |= baudRate;
    }

    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(_uartFilestream, TCIFLUSH);
    tcsetattr(_uartFilestream, TCSANOW, &options);

    //create watchdog
    _serialWatchdog = new WatchDog(std::chrono::microseconds(500000), Serial::OnWatchdogExpire);
    _serialWatchdog->Start(true);
}

Serial::~Serial()
{
	close(_uartFilestream);
}

void Serial::OnWatchdogExpire(WatchDog* self)
{
    Debug::error("Serial communication Timed Out... PLEASE HELP!");
    std::thread(&WatchDog::Stop, self);
    std::thread([self](){
        std::this_thread::sleep_for(1s);
        self->Start(true);
        });
}

HCP_MSG* Serial::ReadSync()
{
    std::lock_guard<std::mutex> lock(serialMtx);
    //----- CHECK FOR ANY RX BYTES -----
    uint8 rxBuffer[MSG_SIZE] = {0};
    //msg.msg = rxBuffer;
    bool finished = false;
    if (_uartFilestream != -1)
    {
        while(!finished)
        {
            _serialWatchdog->Restart();
            int rxLength = read(_uartFilestream, (void *) rxBuffer,
                                1);        //Filestream, buffer to store in, number of bytes to read (MSG_SIZE)

            _serialWatchdog->Pause();
            if (rxLength < 0)
            {
                //NOTE: if this occurs settings of serial com is broken --> non blocking
//                cout << "optcode: no bytes recieved" << endl;
            }
            else if (rxLength == 0)
            {
                //No data waiting
                cout << "optcode: no opcode...waiting" << endl;
                usleep(1);
            }
            else
            {
                HCP_MSG* msg = new HCP_MSG;

                msg->optcode = rxBuffer[0];


                int msgLength = hcp_cmds[msg->optcode].payloadLength;
                Debug::info("Opcode: %x | Message Length: %d", msg->optcode, msgLength);
                int remainingBytes = msgLength;


                msg->payloadSize = msgLength;
                msg->payload = new uint8[msg->payloadSize];

                while (remainingBytes > 0)
                {
                    _serialWatchdog->Restart();
                    rxLength = read(_uartFilestream, (void *) rxBuffer, remainingBytes);
                    _serialWatchdog->Pause();
//                    cout << rxLength << " bytes recieved" << endl;
                    if (rxLength < 0)
                    {
                        //NOTE: if this occurs settings of serial com is broken --> non blocking
//                        cout << "payload: no bytes recieved" << endl;
                    }
                    else if (rxLength == 0)
                    {
                        //No data waiting
                        cout << "payload: no data...waiting" << endl;
                    }
                    else
                    {

                        int32 startIndex = msgLength - remainingBytes;
                        std::copy_n(rxBuffer, rxLength, &(msg->payload[startIndex]));
                        remainingBytes -= rxLength;
                    }
//                    sleep(1);
                }

                finished = true;
                //printf("First Byte in Buffer: %x and in msg: %x\n", rxBuffer[0], msg->payload[0]);
		//printf("---------------\n");
		//printf("0 Byte in msg: %x\n", msg->optcode);
//for (int i = 0; i < msg->payloadSize; i++)
//{
//printf("%d Byte in msg: %x\n", i+1, msg->payload[i]);
//}

                return msg;

            }
        }

    //	sleep(1);
    }
    else
    {
        //Debug::error("Device " + _uartDevice + " disconnected!");
    }
    return nullptr;
}

void Serial::ReadAsync(std::function<void(HCP_MSG)> callback)
{
    std::lock_guard<std::mutex> lock(serialMtx);
    //----- CHECK FOR ANY RX BYTES -----
    uint8 rxBuffer[MSG_SIZE] = {0};
    //msg.msg = rxBuffer;
    bool finished = false;

    if (_uartFilestream != -1)
    {
        while(!finished)
        {
            _serialWatchdog->Restart();
            int rxLength = read(_uartFilestream, (void *) rxBuffer,
                                1);        //Filestream, buffer to store in, number of bytes to read (MSG_SIZE)

            _serialWatchdog->Pause();
            if (rxLength < 0)
            {
                //NOTE: if this occurs settings of serial com is broken --> non blocking
                cout << "optcode: no bytes recieved" << endl;
            }
            else if (rxLength == 0)
            {
                //No data waiting
                cout << "optcode: no data...waiting" << endl;
                usleep(1);
            }
            else
            {
                HCP_MSG msg;

                msg.optcode = rxBuffer[0];


                int msgLength = hcp_cmds[msg.optcode].payloadLength;
                Debug::info("Message Length: %d", msgLength);
                int remainingBytes = msgLength;


                msg.payloadSize = msgLength;
                msg.payload = new uint8[msg.payloadSize];

                while (remainingBytes > 0)
                {
                    _serialWatchdog->Restart();
                    rxLength = read(_uartFilestream, (void *) rxBuffer, remainingBytes);
                    _serialWatchdog->Pause();
//                    cout << rxLength << " bytes recieved" << endl;
                    if (rxLength < 0)
                    {
                        //NOTE: if this occurs settings of serial com is broken --> non blocking
                        cout << "payload: no bytes recieved" << endl;
                    }
                    else if (rxLength == 0)
                    {
                        //No data waiting
                        cout << "payload: no data...waiting" << endl;
                    }
                    else
                    {
                        int32 startIndex = msgLength - remainingBytes;
                        std::copy_n(rxBuffer, rxLength, &(msg.payload[startIndex]));
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
        //Debug::error("Device " + _uartDevice + " disconnected!");
    }
}

void Serial::Write(HCP_MSG message)
{
    std::lock_guard<std::mutex> lock(serialMtx);

    uint8 buffer[message.payloadSize+1];
    buffer[0] = message.optcode;
    Debug::info("Writing Optcode %x...", message.optcode);
    if (message.payloadSize > 0)
    {
        std::copy_n(message.payload, message.payloadSize, &(buffer[1]));
        //std::copy(&message.payload[0], &message.payload[message.payloadSize-1], &buffer[1]);
        Debug::info("first byte: %x", buffer[1]);
    }
    if (_uartFilestream != -1)
    {
        _serialWatchdog->Restart();
        //Filestream, bytes to write, number of bytes to write
        int count = write(_uartFilestream, &buffer[0], message.payloadSize+1);
        _serialWatchdog->Pause();
        if (count < 0)
        {
            printf("UART Write error\n");
        }
    }
    else
    {
        //Debug::error("Device " + _uartDevice + " disconnected!");
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
