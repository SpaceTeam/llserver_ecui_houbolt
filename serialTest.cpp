//
// Created by Markus on 2019-09-29.
//

#include <iostream>

#include "Serial.h"

using namespace std;

void onRead(Hedgehog_Msg msg)
{
//    for (int i = 0; i < msg.size; i++)
//    {
//    	printf("%d", msg.msg[i]);
//    }
//    printf("\n");
    cout << "Optcode: " << msg.optcode << endl;
    cout << msg.msg << endl;
}

int main(int argc, char const *argv[])
{
    Serial* hhgSerial = new Serial("/dev/serial0", 115200);

    char msgArr[] = {0x01};
    Hedgehog_Msg msg;
    msg.size = 1;
    msg.msg = msgArr;
    for (int i = 0; i < 100000; i++)
    {
    	hhgSerial->Write(msg);
        hhgSerial->Read(onRead);
    }
    return 0;
}
