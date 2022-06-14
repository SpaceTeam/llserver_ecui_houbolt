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

#include "driver/UDPSocket.h"

#define HEADER_SIZE 2
#define MAX_MSG_LENGTH 65536

UDPSocket::UDPSocket(std::string name, std::function<void()> onCloseCallback, std::string address, uint16_t port)
{
    this->name =name;
    this->address = address;
    this->port = port;
    this->onCloseCallback = onCloseCallback;

    this->buffer = new uint8_t[MAX_MSG_LENGTH];
    std::fill(buffer, buffer+MAX_MSG_LENGTH, 0);
}

UDPSocket::~UDPSocket()
{
    std::lock_guard<std::mutex> lock(socketMtx);
    this->shallClose = true;

    close(socketfd);
    this->connectionActive = false;
    delete[] this->buffer;
}

int UDPSocket::Connect(int32_t tries)
{

    if ((socketfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        Debug::error("UDPSocket - %s: UDPSocket creation error", name.c_str());
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, address.c_str(), &serv_addr.sin_addr) <= 0)
    {
        close(socketfd);
        Debug::error("UDPSocket - %s:Invalid address/ Address not supported", name.c_str());
        return -2;
    }

    serv_addr_len = sizeof(serv_addr);

    while (!shallClose && tries != 0)
    {
        Debug::print("UDPSocket - %s: Attempting connection...", name.c_str());
        if (connect(socketfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == 0){
            Debug::print("UDPSocket - %s: Connected", name.c_str());
            connectionActive = true;
            return 0;
        }

        tries--;

        if (tries == 0)
        {
            Debug::error("UDPSocket - %s: Couldn't connect to %s PORT: %d\n", name.c_str(), address.c_str(), port);
            return -3;
        }

        sleep(3);
    }

    //Should never reach here
    return -4;

}

void UDPSocket::Send(UDPMessage * msg)
{
    std::lock_guard<std::mutex> lock(socketMtx);
    if (connectionActive)
    {
        int sentBytes = sendto(socketfd, msg->data, msg->dataLength, 0, (sockaddr *)&serv_addr, serv_addr_len);
        if (sentBytes < 0 || sentBytes != (int)(msg->dataLength))
        {
            Debug::error("UDPSocket - %s: error at send occured, closing socket..."), name.c_str();
            Close();
        }
    }
    else
    {
        Debug::error("UDPSocket - %s: no connection active", name.c_str());
    }
}

/**
    Receive a single packet via the TCP connection, packets are expected to havethe following format [HEADER][PAYLOAD]
    The header contains the length of the packet, while the payload contains the json string
*/
void UDPSocket::Recv(UDPMessage *msg)
{
    msg->dataLength = 0;
    if (connectionActive)
    {    
        msg->data = buffer;
        //Receive the header
        msg->dataLength = recvfrom(socketfd, buffer, MAX_MSG_LENGTH, MSG_WAITALL, (sockaddr *)&serv_addr, &serv_addr_len);
        if (msg->dataLength <= 0){
            Debug::error("UDPSocket - %s: error at recv occured (Could not read header), closing socket...", name.c_str());
            this->connectionActive = false;
            //TODO: write better exception
            throw std::runtime_error("UDPSocket error");
        }
    } 
    else
    {
        Debug::error("UDPSocket - %s: no connection active", name.c_str());
        this->connectionActive = false;
        //TODO: write better exception
        throw std::runtime_error("UDPSocket error");
    }
}


// std::vector<uint8_t> UDPSocket::newRecvBytes()
// {
//     std::string msg = Recv();
//     return std::vector<uint8_t>(msg.begin(), msg.end());
// }

bool UDPSocket::isConnectionActive()
{
    return this->connectionActive;
}

void UDPSocket::Close()
{
    std::lock_guard<std::mutex> lock(socketMtx);
    this->shallClose = true;

    close(socketfd);
    this->connectionActive = false;
    this->onCloseCallback();
}