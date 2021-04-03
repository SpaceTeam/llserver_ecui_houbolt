//
// Created by Markus on 15.04.20.
//

#include "drivers/SocketServer.h"

#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>

#include "Debug.h"

using namespace std;

void SocketServer::Open(std::string address, uint16 port)
{
    struct addrinfo hints, *ai;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int res = getaddrinfo(address.c_str(), to_string(port).c_str(), &hints, &ai);
    if (res != 0)
    {
        const char* errorMsg = gai_strerror(res);
        Debug::error(errorMsg);
        return;

    }

    int sockfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if (sockfd < 0)
    {
        Debug::error("create socket server failed");
        freeaddrinfo(ai);;
    }

    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

    int bindVal = ::bind(sockfd, ai->ai_addr, ai->ai_addrlen);
    if (bindVal < 0)
    {
        Debug::error("socket server bind failed");
        close(sockfd);
        freeaddrinfo(ai);;
    }

    if (listen(sockfd, 1) < 0)
    {
        Debug::error("socket server listen failed");
        close(sockfd);
        freeaddrinfo(ai);;
    }

    this->sockfd = sockfd;

}

void SocketServer::AsyncAccept(SocketServer *self)
{
    while (!self->shallClose)
    {
        self->connfd = accept(self->sockfd, NULL, NULL);
        if (self->connfd < 0)
        {
            close(self->sockfd);
            if (self->shallClose)
            {
                Debug::error("abort socket accept and quitting...\n");
                return;
            }
            else
            {
                Debug::error("socket accept failed");
                return;
            }

        }

        self->buffer = new uint8[self->size];
        self->_isConnectionActive = true;
    }

    close(self->sockfd);
    Debug::info("quitting...\n");
}

void SocketServer::Start(string address, uint16 port, std::function<void()> onCloseCallback)
{
    this->onCloseCallback = onCloseCallback;
    Open(address, port);
    if (this->sockfd < 0)
    {
        Debug::error("open socket server Failed");
        return;
    }
    asyncAcceptThread = new thread(AsyncAccept, this);
    asyncAcceptThread->detach();


}

bool SocketServer::isConnectionActive()
{
    return _isConnectionActive;
}

void SocketServer::Send(vector<uint8> msg)
{
    if (_isConnectionActive)
    {
        int sentBytes = write(sockfd, msg.data(), msg.size());
        if (sentBytes < 0)
        {
            Debug::error("error at send occured, closing socket...");
            Stop();
        }
    }
    else
    {
        Debug::error("no connection active");
    }
}

string SocketServer::Recv()
{
    Debug::error("SocketServer Recv not implemented yet");
    return "";
}

vector<uint8> SocketServer::RecvBytes(uint32 sizeInBytes)
{
    vector<uint8> msg;
    bool finished = false;
    if (_isConnectionActive)
    {
        while(!finished)
        {
            int recvBytes = recv(connfd, buffer, sizeInBytes, MSG_WAITALL);        //Filestream, buffer to store in, number of bytes to read (MSG_SIZE)
            if (recvBytes < 0)
            {
                //NOTE: if this occurs settings of serial com is broken --> non blocking
                Debug::error("in ServerSocket: error at recv");
            }
            else if (recvBytes == 0)
            {
                usleep(1);
            }
            else
            {
                if (recvBytes != sizeInBytes)
                {
                    Debug::error("in ServerSocket: requested msg not fully received");
                }
                else
                {
                    msg.resize(recvBytes);
                    std::copy(buffer, buffer+recvBytes, msg.begin());
                    finished = true;
                }
            }
        }
    }
    else
    {
        Debug::error("no connection in SocketServer active");
    }

    return msg;
}

void SocketServer::Stop()
{
    shallClose = true;
    if (_isConnectionActive)
    {
        close(connfd);
    }
    _isConnectionActive = false;
    close(sockfd);
    onCloseCallback();
}

