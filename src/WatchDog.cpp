//
// Created by Markus on 03.12.20.
//

#include "WatchDog.h"

WatchDog::WatchDog(std::chrono::microseconds expireInterval, std::function<void(WatchDog*)> onExpireCallback)
{
    this->expireInterval = expireInterval;
    this->expireCallback = onExpireCallback;
}

WatchDog::~WatchDog()
{
    Stop();
}

void WatchDog::Expired()
{
    this->expireCallback(this);

    //stop watchdog
    expireMtx.lock();
        watching = false;
    expireMtx.unlock();
}

auto WatchDog::GetNextExpireTime()
{
    std::lock_guard<std::mutex> lock(expireMtx);
    return (this->lastPetTime + expireInterval);
}

void WatchDog::WatchLoop()
{
    Debug::print("Watchdog started\n");
    while (watching)
    {
        if (!paused)
        {
            if (!petted)
            {
                Debug::error("Watchdog expired\n");
                Expired();
            }
            expireMtx.lock();
            petted = false;
            expireMtx.unlock();
        }
        if (watching)
        {
            std::this_thread::sleep_until(GetNextExpireTime());
        }

    }
}

void WatchDog::Start(bool startPaused)
{
    paused = startPaused;
    lastPetTime = std::chrono::high_resolution_clock::now();
    watchThread = std::thread(&WatchDog::WatchLoop, this);
}

void WatchDog::Stop()
{
    std::lock_guard<std::mutex> lock(expireMtx);
    if (watching)
    {
        watching = false;
        watchThread.join();
    }
}

void WatchDog::Pause()
{
    if (!paused)
    {
        paused = true;
    }
}

void WatchDog::Restart()
{
    std::lock_guard<std::mutex> lock(expireMtx);
    if (paused)
    {
        lastPetTime = std::chrono::high_resolution_clock::now();
        petted = true;
        paused = false;
    }
}

void WatchDog::Pet()
{
    std::lock_guard<std::mutex> lock(expireMtx);
    lastPetTime = std::chrono::high_resolution_clock::now();
    petted = true;
}
