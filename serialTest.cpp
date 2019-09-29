//
// Created by Markus on 2019-09-29.
//

#include <iostream>

#include "Serial.h"

using namespace std;

void onRead(Hedgehog_Msg msg)
{
    cout << msg.msg << endl;
}

int main(int argc, char const *argv[])
{
    Serial* hhgSerial = new Serial("/dev/ttyAMA0", 115200);

    hhgSerial->Write("hello");
    hhgSerial->Read(onRead);
}
