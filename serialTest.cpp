//
// Created by Markus on 2019-09-29.
//

#include <iostream>

#include "HcpCommands.h"

#include "Serial.h"

using namespace std;

void onRead(Hedgehog_Msg msg)
{
    printf("Optcode: %d\n", msg.optcode);
    
    switch (msg.optcode)
    {
	case HCP_ANALOG_REP:
	{
            int16 val = (msg.msg[1] << 8) + msg.msg[2];
	    printf("Analog Port: %d | Value: %d\n", msg.msg[0], val);
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
   
    	    for (int i = 0; i < msg.size; i++)
    	    {
    	    	printf("%d\n", msg.msg[i]);
    	    }
	    break;
    	}
    }    
}

int main(int argc, char const *argv[])
{
    Serial* hhgSerial = new Serial("/dev/serial0", 115200);

    char msgArr[] = {0x01};
    Hedgehog_Msg msg;
    msg.size = 1;
    msg.msg = msgArr;

    Hedgehog_Msg msg2;
	msg2.size = 2;
	char anaArr[] = {HCP_ANALOG_REQ, 0x01};
	msg2.msg = anaArr;

    Hedgehog_Msg msg3;
    msg3.size = 4;
    char serArr[] = {HCP_SERVO, 0x01, 0, 0};
    msg3.msg = serArr;    
    for (int i = 0; i < 200000; i+= 50)
    {
    //	hhgSerial->Write(msg);
      //  hhgSerial->Read(onRead);

//	uint16 pos = (i % 2500);
//	printf("%d\n", pos);
//	msg3.msg[2] = (pos >> 8) | 0x80;
//	msg3.msg[3] = pos & 0x00FF;  
	hhgSerial->Write(msg2);
	hhgSerial->Read(onRead);
	usleep(100000);
    }
    return 0;
}
