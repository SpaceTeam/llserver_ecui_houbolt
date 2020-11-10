//
// Created by Markus on 2019-09-27.
//

#ifndef TXV_ECUI_LLSERVER_TIMER_H
#define TXV_ECUI_LLSERVER_TIMER_H

#include <thread>
#include <functional>
#include <mutex>
#include "common.h"

#define NSEC_PER_SEC 1000000000LL

class Timer
{

private:

    bool isRunning = false;

    int64 microTime;
    int64 endTime;

    int64 reportedOffset;
    uint64 interval_ns;


    std::thread* timerThread;
    std::mutex syncMtx;
    std::function<void()> stopCallback;
    std::function<void(int64)> tickCallback;

    static void tick(Timer* self, uint64 interval, int64 endTime, int64 microTime);
    static void highPerformanceTimerLoop(Timer* self, uint64 interval, int64 endTime, int64 microTime);
    static void highPerformanceContinousTimerLoop(Timer* self, uint64 interval, int64 microTime);
    static void nullStopCallback() {};

    void internalLoop(void);
    void internalContinousLoop(void);

public:

    Timer();

    void start(int64 startTimeMicros, int64 endTimeMicros, uint64 intervalMicros, std::function<void(int64)> tickCallback, std::function<void()> stopCallback = nullStopCallback);
    void startContinous(int64 startTimeMicros, uint64 intervalMicros, std::function<void(int64)> tickCallback, std::function<void()> stopCallback = nullStopCallback);
    void stop();

    void startContinousNow(uint64 intervalMicros, std::function<void(int64)> tickCallback, std::function<void()> stopCallback);
    static void incrementTimeSpec(struct timespec *ts, long nsec);
    static void normalizeTimestamp(struct timespec *ts);


    ~Timer();

};


#endif //TXV_ECUI_LLSERVER_TIMER_H
