//
// Created by Markus on 2019-09-29.
//

#include <iostream>

#include "HcpCommands.h"

#include "Serial.h"

using namespace std;

void onRead(Hedgehog_Msg msg)
{
    cout << "Optcode: " << msg.optcode << endl;

    for (int i = 0; i < msg.size; i++)
    {
    	printf("%d\n", msg.msg[i]);
    }
    //printf("\n");
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
	sleep(1);
    }
    return 0;
}
