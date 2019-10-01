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

#include "Socket.h"

using namespace std;

int32 Socket::socketfd;
int8 Socket::buffer[SOCKET_MSG_SIZE] = {0};

bool Socket::connectionActive;
bool Socket::shallClose = false;

std::thread* Socket::asyncListenThread;

int Socket::init(std::function<void(int32, json)> onMsgCallback)
{
    struct sockaddr_in serv_addr;

    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, SOCKET_IP, &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(socketfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    connectionActive = true;

    asyncListenThread = new thread(asyncListen, onMsgCallback);

    return 0;
}

void Socket::asyncListen(std::function<void(int32, json)> onMsgCallback)
{
    char buffer[SOCKET_MSG_SIZE] = {0};
    int valread;

    while(!shallClose)
    {
        valread = recv(socketfd, buffer, SOCKET_MSG_SIZE, 0);

        if (valread < 0)
        {
            Debug::error("error at read occured");
            exit(0);

        }
        cout << valread << endl;
        string msg(buffer);
        cout << msg << endl;

        json jsonMsg = json::parse(msg);

        onMsgCallback(socketfd, jsonMsg);

        std::fill(std::begin(buffer), std::end(buffer), 0);

        //sleep(1);
    }
}

void Socket::sendJson(std::string type)
{
    sendJson(type, nullptr);
}

void Socket::sendJson(std::string type, json content)
{
    if (connectionActive)
    {
        json jsonMsg = json::object();
        jsonMsg["type"] = type;

        if (content != nullptr)
        {
            jsonMsg["content"] = content;
        }
        else
        {
            jsonMsg["content"] = json::object();
        }

        string msg = jsonMsg.dump();

        send(socketfd, msg.c_str(), msg.size(), 0);

    }
    else
    {
        Debug::error("no connection active");
    }
}

void Socket::sendJson(std::string type, float content)
{
    if (connectionActive)
    {
        json jsonMsg = json::object();
        jsonMsg["type"] = type;

        jsonMsg["content"] = content;


        string msg = jsonMsg.dump();

        send(socketfd, msg.c_str(), msg.size(), 0);

    }
    else
    {
        Debug::error("no connection active");
    }
}

int32 Socket::readJson()
{
    std::fill(std::begin(buffer), std::end(buffer), 0);
    return recv(socketfd, buffer, SOCKET_MSG_SIZE, 0);
}

void Socket::destroy()
{
    shallClose = true;

    asyncListenThread->join();
    delete asyncListenThread;

    connectionActive = false;

}
