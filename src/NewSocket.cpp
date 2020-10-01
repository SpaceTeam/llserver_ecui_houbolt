//
// Created by Markus on 2019-09-28.
//

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <thread>

#include <iostream>
#include <functional>
#include <algorithm>

#include "NewSocket.h"

using namespace std;

#define HEADER_SIZE 2

NewSocket::NewSocket(std::string name, std::function<void()> onCloseCallback, std::string address, uint16 port)
{
    this->name =name;
    this->address = address;
    this->port = port;
    this->onCloseCallback = onCloseCallback;
}

NewSocket::~NewSocket()
{
    std::lock_guard<std::mutex> lock(socketMtx);
    this->shallClose = true;

    close(socketfd);
    this->connectionActive = false;
}

int NewSocket::Connect(int32 tries)
{

    struct sockaddr_in serv_addr;

    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\nSocket - %s: Socket creation error \n", name.c_str());
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, address.c_str(), &serv_addr.sin_addr) <= 0)
    {
        close(socketfd);
        printf("\nSocket - %s:Invalid address/ Address not supported \n", name.c_str());
        return -2;
    }

    while (!shallClose && tries != 0)
    {
        printf("\nSocket - %s: Attempting connection...\n", name.c_str());
        if (connect(socketfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == 0){
            printf("Socket - %s: Connected \n", name.c_str());
            connectionActive = true;
            return 0;
        }

        tries--;

        if (tries == 0)
        {
            printf("Socket - %s: Couldn't connect to %s PORT: %d\n", name.c_str(), address.c_str(), port);
            return -3;
        }

        sleep(3);
    }

    //Should never reach here
    return -4;

}

void NewSocket::Send(std::string msg)
{
    std::lock_guard<std::mutex> lock(socketMtx);
    if (connectionActive)
    {
        int sentBytes = send(socketfd, msg.c_str(), msg.size(), 0);
        if (sentBytes < 0)
        {
            Debug::error("Socket - %s: error at send occured, closing socket..."), name.c_str();
            Close();
        }
    }
    else
    {
        Debug::error("Socket - %s: no connection active", name.c_str());
    }
}

/**
    Receive a single packet via the TCP connection, packets are expected to havethe following format [HEADER][PAYLOAD]
    The header contains the length of the packet, while the payload contains the json string
*/
std::string NewSocket::Recv()
{
    uint8_t header[HEADER_SIZE];
    uint32 nBytes;

    if(connectionActive){
        
        //Receive the header
        nBytes = recv(socketfd, header, HEADER_SIZE, MSG_WAITALL);
        if(nBytes < HEADER_SIZE){
            Debug::error("Socket - %s: error at recv occured (Could not read header), closing socket...", name.c_str());
            Close();
            return std::string("");
        }

        //Prepare to receive the payload
        uint32 msgLen;
        msgLen  = header[1];
        msgLen += header[0] << 8;

        uint8_t newBuffer[msgLen+1];
        newBuffer[msgLen] = 0; //Strings are stupid

        //Receive the payload
        nBytes = recv(socketfd, newBuffer, msgLen, MSG_WAITALL);
        if(nBytes < msgLen){
            Debug::error("Socket - %s: error at recv occured (Could not read entire packet), closing socket...", name.c_str());
            Close();
            return std::string("");
        }

        return std::string((char *)newBuffer);
    }else{
        Debug::error("Socket - %s: no connection active", name.c_str());
        Close();
        return std::string("");
    }
}


// std::vector<uint8> Socket::newRecvBytes()
// {
//     std::string msg = Recv();
//     return std::vector<uint8>(msg.begin(), msg.end());
// }

bool NewSocket::isConnectionActive()
{
    return this->connectionActive;
}

void NewSocket::Close()
{
    std::lock_guard<std::mutex> lock(socketMtx);
    this->shallClose = true;

    close(socketfd);
    this->connectionActive = false;
    this->onCloseCallback();
}