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
// #define ENABLE_TIMER_DIAGNOSTICS

class Timer
{

private:

    int threadPriority;
    bool isRunning = false;

    std::string threadName;
    static int nameIndex;

    int64_t microTime;
    int64_t endTime;

    int64_t reportedOffset;
    uint64_t interval_ns;

#ifdef ENABLE_TIMER_DIAGNOSTICS
    long max_exec_time=0;
    const long report_loops = 100;
    long loops_cnt=0;
#endif


    std::thread* timerThread;
    std::mutex syncMtx;
    std::function<void()> stopCallback;
    std::function<void(int64_t)> tickCallback;

    static void tick(Timer* self, uint64_t interval, int64_t endTime, int64_t microTime);
    static void highPerformanceTimerLoop(Timer* self, uint64_t interval, int64_t endTime, int64_t microTime);
    static void highPerformanceContinousTimerLoop(Timer* self, uint64_t interval, int64_t microTime);
    static void nullStopCallback() {};

    void internalLoop(void);
    void internalContinousLoop(void);

public:

    Timer(int prio=40, std::string threadName="Thread"+std::to_string(nameIndex++));

    void start(int64_t startTimeMicros, int64_t endTimeMicros, uint64_t intervalMicros, std::function<void(int64_t)> tickCallback, std::function<void()> stopCallback = nullStopCallback);
    void startContinous(int64_t startTimeMicros, uint64_t intervalMicros, std::function<void(int64_t)> tickCallback, std::function<void()> stopCallback = nullStopCallback);
    void stop();

    void startContinousNow(uint64_t intervalMicros, std::function<void(int64_t)> tickCallback, std::function<void()> stopCallback);
    void incrementTimeSpec(struct timespec *ts, uint64_t nsec, struct timespec *tsAfter);
    void normalizeTimestamp(struct timespec *ts);
    void diffTimeSpec(struct timespec *ts, struct timespec *ts2, struct timespec *result);


    ~Timer();

};


#endif //TXV_ECUI_LLSERVER_TIMER_H
