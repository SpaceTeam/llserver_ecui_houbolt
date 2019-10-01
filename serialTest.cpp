//
// Created by Markus on 2019-09-29.
//

#include <iostream>

#include "HcpCommands.h"
#include "HcpManager.h"

#include "Serial.h"


using namespace std;

void onRead(HCP_MSG msg)
{
    printf("Optcode: %d\n", msg.optcode);
    
    switch (msg.optcode)
    {
	case HCP_ANALOG_REP:
	{
            int16 val = (msg.payload[1] << 8) + msg.payload[2];
	    printf("Analog Port: %d | Value: %d\n", msg.payload[0], val);
	    break;
	}
	case HCP_INVALID_PORT:
	{
	    printf("Invalid Port \n");
	    break;
	}
	default:
	{
	    cout << "Msg not implemented, printing bytes" << endl; 
   
    	    for (int i = 0; i < msg.payloadSize; i++)
    	    {
    	    	printf("%d\n", msg.payload[i]);
    	    }
	    break;
    	}
    }    
}

int main(int argc, char const *argv[])
{
    /*Serial* hhgSerial = new Serial("/dev/serial0", 115200);

    HCP_MSG msg;
    msg.optcode = HCP_VERS_REQ;
    msg.payloadSize = 0;
    msg.payload = {};

    HCP_MSG msg2;
	msg2.payloadSize = 1;
	msg2.optcode = HCP_ANALOG_REQ;
	uint8 anaArr[] = {HCP_ANALOG_REQ, 0x01};
	msg2.payload = anaArr;

    HCP_MSG msg3;
    msg3.payloadSize = 3;
    msg3.optcode = HCP_SERVO;
    uint8 serArr[] = {0x01, 0, 0};
    msg3.payload = serArr;

    for (int i = 0; i < 200000; i+= 50)
    {
    //	hhgSerial->Write(msg);
      //  hhgSerial->ReadAsync(onRead);

	uint16 pos = (i % 2500);
	printf("%d\n", pos);
	msg3.payload[1] = (pos >> 8) | 0x80;
	msg3.payload[2] = pos & 0x00FF;
	hhgSerial->Write(msg3);
	hhgSerial->ReadAsync(onRead);
	usleep(100000);
    }
    return 0;*/

//    string fuel = "fuel";
//    string analog = "fuel feedback";
//    HcpManager::init();
//
//    HcpManager::EnableServo(0);
//    HcpManager::SetServo(fuel, 200);
//    sleep(1);
//    HcpManager::SetServoRaw(0, 200);
//    sleep(1);
//    HcpManager::SetServo(fuel, 80);
//    sleep(1);
//    HcpManager::SetServoRaw(0, 0);
//
//    for (int i = 0; i < 100; i++)
//    {
//        cout << HcpManager::GetAnalog(analog) << endl;
//    	usleep(50000);
//    }
//    HcpManager::DisableServo(0);

    HcpManager::init();

    HcpManager::EnableServo(0);

    string fuel = "fuel";
    string analog = "fuel feedback";
    string digital = "safety";
    for (int i = 0; i < 8000; i++)
    {
        if (!HcpManager::ExecCommand(fuel, i))
        {
            cout << "not working" << endl;
            break;
        }
        cout << HcpManager::GetAnalog(analog) << endl;
        cout << HcpManager::GetDigital(digital) << endl;

        usleep(10000);
    }
    HcpManager::DisableServo(0);
}
