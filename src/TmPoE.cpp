//
// Created by Markus on 05.12.19.
//

#include "TmPoE.h"
#include "Config.h"

using namespace std;

TMPoE::TMPoE(uint16 id, uint32 sampleRate)
{
    this->id = id;
    std::string ip = std::get<std::string>(Config::getData("TMPoE/ip"));
    int32 port = std::get<int>(Config::getData("TMPoE/port"));
    socket = new Socket(OnClose, ip, port, 1);
    if (socket->isConnectionActive())
    {
        currValues.resize(8, -1);
        asyncListenThread = new thread(AsyncListen, this, sampleRate);
        asyncListenThread->detach();
    }
}

TMPoE::~TMPoE()
{
    shallClose = true;
    delete socket;
}

void TMPoE::AsyncListen(TMPoE *self, uint32 sampleRate)
{
    uint32 sleepDuration = 1.0/sampleRate*1000000.0;
    while(!self->shallClose)
    {
        self->socket->Send("temp\n");
        vector<uint8> msg = self->socket->RecvBytes();
        if (msg.size() < 24)
        {
            Debug::error("TMPoE response message is too small");
            continue;
        }

        self->readMtx.lock();
        for (int i = 0; i < self->currValues.size(); i++)
        {
            self->currValues[i] = (msg[i*3] << 16) | (msg[i*3+1] << 8) | msg[i*3+2];
        }
        self->readMtx.unlock();

        usleep(sleepDuration);
    }
}

std::vector<uint32> TMPoE::Read()
{
    std::lock_guard<std::mutex> lock(readMtx);
    return this->currValues;
}

//TODO: make Socket class to be able to parse current object as argument
void TMPoE::OnClose()
{

}