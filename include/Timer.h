//
// Created by Markus on 2019-09-27.
//

#ifndef TXV_ECUI_LLSERVER_TIMER_H
#define TXV_ECUI_LLSERVER_TIMER_H

#include <thread>
#include <functional>
#include "common.h"

class Timer
{

private:

    bool isRunning = false;

    int64 microTime;

    int64 startTime;
    int64 endTime;
    uint64 interval;


    std::thread* timerThread;
    std::function<void(int64)> tickCallback;
    std::function<void()> stopCallback;

    static void tick(Timer* self, uint64 interval, int64 endTime, int64 microTime);

public:

    Timer();

    void start(int64 startTimeMicros, int64 endTimeMicros, uint64 intervalMicros, std::function<void(int64)> tickCallback, std::function<void()> stopCallback);
    void stop();


    ~Timer();

};


#endif //TXV_ECUI_LLSERVER_TIMER_H
