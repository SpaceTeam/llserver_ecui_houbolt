//
// Created by Markus on 03.12.20.
//

#ifndef TXV_ECUI_LLSERVER_WATCHDOG_H
#define TXV_ECUI_LLSERVER_WATCHDOG_H

#include <chrono>

#include "common.h"

//if pause and restart functions are not used, behaves like a normal periodic watchdog timer
//if watchdog is started with pause option, it can be resumed with Restart();
class WatchDog
{

private:
    std::chrono::microseconds expireInterval;
    std::thread watchThread;
    std::function<void(WatchDog*)> expireCallback;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastPetTime;

    std::mutex expireMtx;

    bool watching = false;
    bool petted = true;
    bool paused = false;

    void Expired();

    void WatchLoop();

    auto GetNextExpireTime();
public:

    WatchDog(std::chrono::microseconds expireInterval, std::function<void(WatchDog*)> onExpireCallback);

    ~WatchDog();

    void Start(bool startPaused = false);
    void Stop();
    //Creates a one shot functionality, can be restarted with Restart() function
    void Pause();
    //resets timer, same as start, but doesn't create new thread to increase performance
    void Restart();
    void Pet();


};


#endif //TXV_ECUI_LLSERVER_WATCHDOG_H
