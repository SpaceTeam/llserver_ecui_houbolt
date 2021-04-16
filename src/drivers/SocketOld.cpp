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

#include "driver/SocketOld.h"

using namespace std;

#define HEADER_SIZE 2

SocketOld::SocketOld(std::function<void()> onCloseCallback, std::string address, uint16_t port, int32_t tries)
{
    buffer = new uint8_t[size];
    this->onCloseCallback = onCloseCallback;
    Connect(address, port, tries);
}

SocketOld::~SocketOld()
{
    std::lock_guard<std::mutex> lock(socketMtx);
    this->shallClose = true;

    close(socketfd);
    this->connectionActive = false;
    delete buffer;
}

void SocketOld::Connect(std::string address, uint16_t port, int32_t tries)
{

    struct sockaddr_in serv_addr;
    int32_t totalTries = tries;

    while (!connectionActive && !shallClose && tries != 0)
    {
        if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            Debug::error("SocketOld creation error");
            continue;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);

        // Convert IPv4 and IPv6 addresses from text to binary form
        if (inet_pton(AF_INET, address.c_str(), &serv_addr.sin_addr) <= 0)
        {
            Debug::error("SocketOld - %s:Invalid address/ Address not supported");
            continue;
        }

        if (connect(socketfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            Debug::print("Waiting for connection...");
            if (totalTries > 1 || totalTries < 0)
            {
                sleep(3);
            }
            tries -= 1;
            this_thread::yield();
            continue;

        }

        connectionActive = true;
        Debug::print("Connected");
    }
    if (tries == 0)
    {
        Debug::error("Couldn't connect to %s PORT: %d\n", address.c_str(), port);
    }
}

void SocketOld::Send(std::string msg)
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
        Debug::info("no connection active");
    }
}

/**
    Receive a single packet via the TCP connection, packets are expected to havethe following format [HEADER][PAYLOAD]
    The header contains the length of the packet, while the payload contains the json string
*/
std::string SocketOld::newRecv()
{
    uint8_t header[HEADER_SIZE];
    uint32_t nBytes;

    if(connectionActive){

        //Receive the header
        nBytes = recv(socketfd, header, HEADER_SIZE, MSG_WAITALL);
        if(nBytes < HEADER_SIZE){
            Debug::error("error at recv occured (Could not read header), closing socket...");
            Close();
        }

        //Prepare to receive the payload
        uint32_t msgLen;
        msgLen  = header[1];
        msgLen += header[0] << 8;

        printf("Expecting a message of length %u, (%u, %u)\n", msgLen, header[0], header[1]);
        uint8_t newBuffer[msgLen+1];
        newBuffer[msgLen] = 0; //Strings are stupid

        //Receive the payload
        nBytes = recv(socketfd, newBuffer, msgLen, MSG_WAITALL);
        if(nBytes < msgLen){
            Debug::error("error at recv occured (Could not read entire packet), closing socket...");
            Close();
        }

        printf("=== MSG ===\n%s\n=======\n",(char *)newBuffer);

        return std::string((char *)newBuffer);
    }else{
        Debug::info("no connection active");
        return std::string("");
    }
}


// std::vector<uint8_t> SocketOld::newRecvBytes()
// {
//     std::string msg = Recv();
//     return std::vector<uint8_t>(msg.begin(), msg.end());
// }

std::string SocketOld::Recv()
{

    string msg = "";
    if (connectionActive)
    {
        int32_t valread;

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
        Debug::info("no connection active");
    }
    return msg;
}

std::vector<uint8_t> SocketOld::RecvBytes()
{
    std::vector<uint8_t> msg;
    if (connectionActive)
    {
        int32_t valread;

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
        Debug::info("no connection active");
    }
    return msg;
}

bool SocketOld::isConnectionActive()
{
    return this->connectionActive;
}

void SocketOld::Close()
{
    std::lock_guard<std::mutex> lock(socketMtx);
    this->shallClose = true;

    close(socketfd);
    this->connectionActive = false;
    this->onCloseCallback();
}

//
//
//void SocketOld::asyncListen(std::function<void(int32_t, nlohmann::json)> onMsgCallback)
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
//        nlohmann::json jsonMsg = nlohmann::json::parse(msg);
//
//        onMsgCallback(socketfd, jsonMsg);
//
//        std::fill(std::begin(buffer), std::end(buffer), 0);
//
//        //sleep(1);
//    }
//}
//
//void SocketOld::sendJson(std::string type)
//{
//    sendJson(type, nullptr);
//}
//
//void SocketOld::sendJson(std::string type, nlohmann::json content)
//{
//    if (connectionActive)
//    {
//        sendMtx.lock();
//
//        nlohmann::json jsonMsg = nlohmann::json::object();
//        jsonMsg["type"] = type;
//
//        if (content != nullptr)
//        {
//            jsonMsg["content"] = content;
//        }
//        else
//        {
//            jsonMsg["content"] = nlohmann::json::object();
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
//void SocketOld::sendJson(std::string type, float content)
//{
//    if (connectionActive)
//    {
//        sendMtx.lock();
//
//        nlohmann::json jsonMsg = nlohmann::json::object();
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
//int32_t SocketOld::readJson()
//{
//    std::lock_guard<std::mutex> lock(sendMtx);
//    std::fill(std::begin(buffer), std::end(buffer), 0);
//    return recv(socketfd, buffer, SOCKET_MSG_SIZE, 0);
//}
//
//void SocketOld::destroy()
//{
//    shallClose = true;
//
//    asyncListenThread->join();
//    delete asyncListenThread;
//
//    connectionActive = false;
//
//}