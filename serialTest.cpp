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
            int16 val = msg.msg[1] << 8 + msg.msg[2];
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

    for (int i = 0; i < 10000; i++)
    {
    //	hhgSerial->Write(msg);
      //  hhgSerial->Read(onRead);


	hhgSerial->Write(msg2);
	hhgSerial->Read(onRead);
//	sleep(1);
    }
    return 0;
}
