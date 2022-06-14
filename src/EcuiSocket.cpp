//
// Created by Markus on 2019-10-15.
//

#include <thread>

#include "driver/Socket.h"

#include "EcuiSocket.h"

#include "utility/Config.h"

Socket* EcuiSocket::socket;

std::atomic_bool EcuiSocket::connectionActive = false;
std::atomic_bool EcuiSocket::shallClose = false;

std::thread* EcuiSocket::asyncListenThread;
std::function<void()> EcuiSocket::onCloseCallback;

void EcuiSocket::Init(std::function<void(nlohmann::json)> onMsgCallback, std::function<void()> onCloseCallback)
{
    EcuiSocket::onCloseCallback = onCloseCallback;

    std::string ip = std::get<std::string>(Config::getData("WEBSERVER/ip"));
    int32_t port = std::get<int>(Config::getData("WEBSERVER/port"));

    socket = new Socket("EcuiSocket", Close, ip, port);
    while(socket->Connect()!=0);
    
    connectionActive = true;
    asyncListenThread = new std::thread(AsyncListen, onMsgCallback);
    asyncListenThread->detach();

}

void EcuiSocket::AsyncListen(std::function<void(nlohmann::json)> onMsgCallback)
{
    while(!shallClose)
    {
        std::string msg;
        try {
            msg = socket->Recv();
            nlohmann::json jsonMsg = nlohmann::json::parse(msg);
            onMsgCallback(jsonMsg);
        } catch (const std::exception& e) {
			std::cerr << e.what();
            Debug::error("nlohmann::json message of Webserver is invalid:\n" + msg);
            socket->Connect();
        }

        std::this_thread::yield();
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

void EcuiSocket::SendJson(std::string type, nlohmann::json content)
{

    if (connectionActive)
    {

        nlohmann::json jsonMsg = nlohmann::json::object();
        jsonMsg["type"] = type;

        if (content != nullptr)
        {
            jsonMsg["content"] = content;
        }
        else
        {
            jsonMsg["content"] = nlohmann::json::object();
        }
        //    cout << "Content: "<< content.dump() << endl;
        //    cout << "Msg: "<< jsonMsg.dump() << endl;
        std::string msg = jsonMsg.dump() + "\n";

//        std::thread sendThread(SendAsync, msg);
//
//        sendThread.detach();
        socket->Send(msg);
    }
    else
    {
        Debug::error("no EcuiSocket connection active");
    }

}

void EcuiSocket::SendJson(std::string type, float content)
{
    if (connectionActive)
    {

        nlohmann::json jsonMsg = nlohmann::json::object();
        jsonMsg["type"] = type;

        jsonMsg["content"] = content;

        //	cout << "Content: "<< content.dump() << endl;
        //	cout << "Msg: "<< jsonMsg.dump() << endl;
        std::string msg = jsonMsg.dump() + "\n";


//        std::thread sendThread(SendAsync, msg);
//        sendThread.detach();
        socket->Send(msg);
    }
    else
    {
        Debug::error("no EcuiSocket connection active");
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
        if (asyncListenThread->joinable())
        {
            asyncListenThread->join();
            delete asyncListenThread;
        }
        
        delete socket;
        connectionActive = false;
    }

}
