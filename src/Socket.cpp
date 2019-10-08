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
std::mutex Socket::sendMtx;

int Socket::init(std::function<void(int32, json)> onMsgCallback)
{
    struct sockaddr_in serv_addr;

    while (!connectionActive)
    {
        if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("\n Socket creation error \n");
            continue;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);

        // Convert IPv4 and IPv6 addresses from text to binary form
        if (inet_pton(AF_INET, SOCKET_IP, &serv_addr.sin_addr) <= 0)
        {
            printf("\nInvalid address/ Address not supported \n");
            continue;
        }

        if (connect(socketfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            printf("\nWaiting for connection...\n");
            sleep(3);
            continue;

        }

        connectionActive = true;
    }
    printf("Connected \n");

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
//        cout << valread << endl;
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
        sendMtx.lock();

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
//	cout << "Content: "<< content.dump() << endl;
//	cout << "Msg: "<< jsonMsg.dump() << endl;
        string msg = jsonMsg.dump() + "\n";

        send(socketfd, msg.c_str(), msg.size(), 0);
        sendMtx.unlock();

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
        sendMtx.lock();

        json jsonMsg = json::object();
        jsonMsg["type"] = type;

        jsonMsg["content"] = content;

//	cout << "Content from float: "<< content << " Type: " << type << endl;

        string msg = jsonMsg.dump() + "\n";

        send(socketfd, msg.c_str(), msg.size(), 0);
        sendMtx.unlock();
    }
    else
    {
        Debug::error("no connection active");
    }
}

int32 Socket::readJson()
{
    std::lock_guard<std::mutex> lock(sendMtx);
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
