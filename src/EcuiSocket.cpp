//
// Created by Markus on 2019-10-15.
//

#include <thread>

#include "NewSocket.h"

#include "EcuiSocket.h"

#include "Config.h"

using namespace std;

NewSocket* EcuiSocket::socket;

bool EcuiSocket::connectionActive = false;
bool EcuiSocket::shallClose = false;

std::thread* EcuiSocket::asyncListenThread;
std::function<void()> EcuiSocket::onCloseCallback;

void EcuiSocket::Init(std::function<void(json)> onMsgCallback, std::function<void()> onCloseCallback)
{
    EcuiSocket::onCloseCallback = onCloseCallback;

    string ip = std::get<std::string>(Config::getData("WEBSERVER/ip"));
    int32 port = std::get<int>(Config::getData("WEBSERVER/port"));

    socket = new NewSocket("EcuiSocket", Close, ip, port);
    while(socket->Connect()!=0);
    
    connectionActive = true;
    asyncListenThread = new thread(AsyncListen, onMsgCallback);
    asyncListenThread->detach();

}

void EcuiSocket::AsyncListen(std::function<void(json)> onMsgCallback)
{
    while(!shallClose)
    {
        string msg;
        try {
            msg = socket->Recv();
            json jsonMsg = json::parse(msg);
            onMsgCallback(jsonMsg);
        } catch (const std::exception& e) {
            Debug::error("json message of Webserver is invalid:\n" + msg);
            socket->Connect();
        }

        this_thread::yield();
    }
}

void EcuiSocket::SendAsync(std::string msg)
{
    socket->Send(msg);
}

void EcuiSocket::SendJson(std::string type)
{
    SendJson(type, nullptr);
}

void EcuiSocket::SendJson(std::string type, json content)
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
        //    cout << "Content: "<< content.dump() << endl;
        //    cout << "Msg: "<< jsonMsg.dump() << endl;
        string msg = jsonMsg.dump() + "\n";

//        std::thread sendThread(SendAsync, msg);
//
//        sendThread.detach();
        socket->Send(msg);
    }
    else
    {
        Debug::error("no connection active");
    }

}

void EcuiSocket::SendJson(std::string type, float content)
{
    if (connectionActive)
    {

        json jsonMsg = json::object();
        jsonMsg["type"] = type;

        jsonMsg["content"] = content;

        //	cout << "Content: "<< content.dump() << endl;
        //	cout << "Msg: "<< jsonMsg.dump() << endl;
        string msg = jsonMsg.dump() + "\n";


//        std::thread sendThread(SendAsync, msg);
//        sendThread.detach();
        socket->Send(msg);
    }
    else
    {
        Debug::error("no connection active");
    }
}

void EcuiSocket::Close()
{
    Destroy();
    onCloseCallback();
}

void EcuiSocket::Destroy()
{
    if (connectionActive)
    {
        shallClose = true;
        delete asyncListenThread;
        delete socket;
        connectionActive = false;
    }

}
