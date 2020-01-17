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

Socket::Socket(std::function<void()> onCloseCallback, std::string address, uint16 port, int32 tries)
{
    buffer = new uint8[size];
    this->onCloseCallback = onCloseCallback;
    Connect(address, port, tries);
}

Socket::~Socket()
{
    std::lock_guard<std::mutex> lock(socketMtx);
    this->shallClose = true;

    close(socketfd);
    this->connectionActive = false;
    delete buffer;
}

void Socket::Connect(std::string address, uint16 port, int32 tries)
{

    struct sockaddr_in serv_addr;
    int32 totalTries = tries;

    while (!connectionActive && !shallClose && tries != 0)
    {
        if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("\n Socket creation error \n");
            continue;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);

        // Convert IPv4 and IPv6 addresses from text to binary form
        if (inet_pton(AF_INET, address.c_str(), &serv_addr.sin_addr) <= 0)
        {
            printf("\nInvalid address/ Address not supported \n");
            continue;
        }

        if (connect(socketfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            printf("\nWaiting for connection...\n");
            if (totalTries > 1 || totalTries < 0)
            {
                sleep(3);
            }
            tries -= 1;
            continue;

        }

        connectionActive = true;
        printf("Connected \n");
    }
    if (tries == 0)
    {
        printf("Couldn't connect to %s PORT: %d\n", address.c_str(), port);
    }
}

void Socket::Send(std::string msg)
{
    std::lock_guard<std::mutex> lock(socketMtx);
    if (connectionActive)
    {
        int sentBytes = send(socketfd, msg.c_str(), msg.size(), 0);
        if (sentBytes < 0)
        {
            Debug::error("error at send occured, closing socket...");
            Close();
        }
    }
    else
    {
        Debug::error("no connection active");
    }
}

std::string Socket::Recv()
{

    string msg = "";
    if (connectionActive)
    {
        int32 valread;

        valread = recv(socketfd, buffer, size, 0);

        if (valread < 0)
        {
            Debug::error("error at recv occured, closing socket...");
            Close();
        }
        cout << "recv" << endl;
        msg = string((char*)buffer);
        cout << msg << endl;

        std::fill(buffer, buffer+size-1, 0);
    }
    else
    {
        Debug::error("no connection active");
    }
    return msg;
}

std::vector<uint8> Socket::RecvBytes()
{
    std::vector<uint8> msg;
    if (connectionActive)
    {
        int32 valread;

        valread = recv(socketfd, buffer, size, 0);

        if (valread < 0)
        {
            Debug::error("error at recv occured, closing socket...");
            Close();
        }

        msg.resize(valread);
        std::copy(buffer, buffer+valread, msg.begin());

        std::fill(buffer, buffer+size-1, 0);
    }
    else
    {
        Debug::error("no connection active");
    }
    return msg;
}

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

//
//
//void Socket::asyncListen(std::function<void(int32, json)> onMsgCallback)
//{
//    char buffer[SOCKET_MSG_SIZE] = {0};
//    int valread;
//
//    while(!shallClose)
//    {
//        valread = recv(socketfd, buffer, SOCKET_MSG_SIZE, 0);
//
//        if (valread < 0)
//        {
//            Debug::error("error at read occured");
//            exit(0);
//
//        }
////        cout << valread << endl;
//        string msg(buffer);
//        cout << msg << endl;
//
//        json jsonMsg = json::parse(msg);
//
//        onMsgCallback(socketfd, jsonMsg);
//
//        std::fill(std::begin(buffer), std::end(buffer), 0);
//
//        //sleep(1);
//    }
//}
//
//void Socket::sendJson(std::string type)
//{
//    sendJson(type, nullptr);
//}
//
//void Socket::sendJson(std::string type, json content)
//{
//    if (connectionActive)
//    {
//        sendMtx.lock();
//
//        json jsonMsg = json::object();
//        jsonMsg["type"] = type;
//
//        if (content != nullptr)
//        {
//            jsonMsg["content"] = content;
//        }
//        else
//        {
//            jsonMsg["content"] = json::object();
//        }
////	cout << "Content: "<< content.dump() << endl;
////	cout << "Msg: "<< jsonMsg.dump() << endl;
//        string msg = jsonMsg.dump() + "\n";
//
//        send(socketfd, msg.c_str(), msg.size(), 0);
//        sendMtx.unlock();
//
//    }
//    else
//    {
//        Debug::error("no connection active");
//    }
//}
//
//void Socket::sendJson(std::string type, float content)
//{
//    if (connectionActive)
//    {
//        sendMtx.lock();
//
//        json jsonMsg = json::object();
//        jsonMsg["type"] = type;
//
//        jsonMsg["content"] = content;
//
////	cout << "Content from float: "<< content << " Type: " << type << endl;
//
//        string msg = jsonMsg.dump() + "\n";
//
//        send(socketfd, msg.c_str(), msg.size(), 0);
//        sendMtx.unlock();
//    }
//    else
//    {
//        Debug::error("no connection active");
//    }
//}
//
//int32 Socket::readJson()
//{
//    std::lock_guard<std::mutex> lock(sendMtx);
//    std::fill(std::begin(buffer), std::end(buffer), 0);
//    return recv(socketfd, buffer, SOCKET_MSG_SIZE, 0);
//}
//
//void Socket::destroy()
//{
//    shallClose = true;
//
//    asyncListenThread->join();
//    delete asyncListenThread;
//
//    connectionActive = false;
//
//}
