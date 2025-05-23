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

#include "driver/Socket.h"

using namespace std;

#define HEADER_SIZE 2
#define MAX_MSG_LENGTH 65536

Socket::Socket(std::string name, std::function<void()> onCloseCallback, std::string address, uint16_t port)
{
    this->name =name;
    this->address = address;
    this->port = port;
    this->onCloseCallback = onCloseCallback;
}

Socket::~Socket()
{
    std::lock_guard<std::mutex> lock(socketMtx);
    this->shallClose = true;

    close(socketfd);
    this->connectionActive = false;
}

int Socket::Connect(int32_t tries)
{

    sockaddr_in serv_addr{};

    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        Debug::error("Socket - %s: Socket creation error", name.c_str());
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, address.c_str(), &serv_addr.sin_addr) <= 0)
    {
        close(socketfd);
        Debug::error("Socket - %s:Invalid address/ Address not supported", name.c_str());
        return -2;
    }

    while (!shallClose && tries != 0)
    {
        Debug::print("Socket - %s: Attempting connection...", name.c_str());
        if (connect(socketfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == 0){
            Debug::print("Socket - %s: Connected", name.c_str());
            connectionActive = true;
            return 0;
        }

        tries--;

        if (tries == 0)
        {
            Debug::error("Socket - %s: Couldn't connect to %s PORT: %d\n", name.c_str(), address.c_str(), port);
            return -3;
        }

        sleep(3);
    }

    //Should never reach here
    return -4;

}

void Socket::Send(const std::string& msg)
{
    std::lock_guard<std::mutex> lock(socketMtx);
    if (connectionActive)
    {
        if (msg.size() > MAX_MSG_LENGTH)
        {
            Debug::error("Socket - %s: error message longer than supported, closing socket...", name.c_str());
            Close();
        }
        else
        {

            uint16_t msgLen = msg.size();
            uint8_t header[HEADER_SIZE];
            header[0] = msgLen >> 8;
            header[1] = msgLen & 0x00FF;
            //Debug::info("msb: 0x%02x, lsb: 0x%02x", header[0], header[1]);
            int sentHeaderBytes = send(socketfd, header, HEADER_SIZE, 0);
            if (sentHeaderBytes < 0 || sentHeaderBytes != HEADER_SIZE)
            {
                Debug::error("Socket - %s: error at send occured, closing socket...", name.c_str());
                Close();
                return;
            }
            int sentBytes = send(socketfd, msg.c_str(), msg.size(), 0);
            if (sentBytes < 0 || sentBytes != (int)(msg.size()))
            {
                Debug::error("Socket - %s: error at send occured, closing socket...",name.c_str()) ;
                Close();
            }
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
std::string Socket::Recv()
{
    uint8_t header[HEADER_SIZE];
    uint32_t nBytes;

    if(connectionActive){
        
        //Receive the header
        nBytes = recv(socketfd, header, HEADER_SIZE, MSG_WAITALL);
        if(nBytes < HEADER_SIZE){
            Debug::error("Socket - %s: error at recv occured (Could not read header), closing socket...", name.c_str());
            this->connectionActive = false;
            //TODO: write better exception
            throw std::runtime_error("Socket error");
        }

        //Prepare to receive the payload
        uint16_t msgLen;
        msgLen  = header[1];
        msgLen += header[0] << 8;
        //Debug::info("MSB: %d, LSB: %d", header[0], header[1]);

        uint8_t newBuffer[msgLen+1];
        newBuffer[msgLen] = 0; //Strings are stupid

        //Receive the payload
        nBytes = recv(socketfd, newBuffer, msgLen, MSG_WAITALL);
        Debug::info("First Message Byte: %d", newBuffer[0]);
        if(nBytes < msgLen){
            Debug::error("Socket - %s: error at recv occured (Could not read entire packet), closing socket...", name.c_str());
            this->connectionActive = false;
            //TODO: write better exception
            throw std::runtime_error("Socket error");
        }

        return std::string((char *)newBuffer);
    }else{
        Debug::error("Socket - %s: no connection active", name.c_str());
        this->connectionActive = false;
        //TODO: write better exception
        throw std::runtime_error("Socket error");
    }
}


// std::vector<uint8_t> Socket::newRecvBytes()
// {
//     std::string msg = Recv();
//     return std::vector<uint8_t>(msg.begin(), msg.end());
// }

bool Socket::isConnectionActive()
{
    return this->connectionActive;
}

void Socket::Close()
{
    std::lock_guard<std::mutex> lock(socketMtx);
    this->shallClose = true;

    close(socketfd);
    this->connectionActive = false;
    this->onCloseCallback();
}