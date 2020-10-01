//
// Created by Markus on 2019-09-27.
//


#include "Timer.h"

#include <chrono>
#include <iostream>
#include <sys/time.h>

typedef std::chrono::high_resolution_clock Clock;

#define TS_TO_MILLI(x) ((x.tv_nsec / 1000)+(x.tv_sec*1000000))

Timer::Timer()
{

    this->microTime = 0;

}

Timer::~Timer()
{

    if (this->isRunning)
    {
        stop();
    }

}

void Timer::start(int64 startTimeMicros, int64 endTimeMicros, uint64 intervalMicros, std::function<void(int64)> tickCallback, std::function<void()> stopCallback)
{
    if (!this->isRunning)
    {
        this->interval_ns = intervalMicros*1000;
        this->reportedOffset = startTimeMicros;
        this->endTime = endTimeMicros;

        this->tickCallback = tickCallback;
        this->stopCallback = stopCallback;

        this->isRunning = true;
        this->timerThread = new std::thread(&Timer::internalLoop, this);
        this->timerThread->detach();
    }
    else
    {
        Debug::error("timer already running");
    }
}

// void Timer::start(int64 startTimeMicros, int64 endTimeMicros, uint64 intervalMicros, std::function<void(int64)> tickCallback, std::function<void()> stopCallback)
// {
//     if (!this->isRunning)
//     {

//         this->startTime = startTimeMicros;
//         this->endTime = endTimeMicros;
//         this->interval = intervalMicros;
//         this->tickCallback = tickCallback;
//         this->stopCallback = stopCallback;

//         this->microTime = startTimeMicros;

//         this->isRunning = true;
//         this->timerThread = new std::thread(Timer::highPerformanceTimerLoop, this, intervalMicros, endTimeMicros, startTimeMicros);
//         this->timerThread->detach();
//     }
//     else
//     {
//         Debug::error("timer already running");
//     }
// }

void Timer::startContinous(int64 startTimeMicros, uint64 intervalMicros, std::function<void(int64)> tickCallback, std::function<void()> stopCallback)
{
    if (!this->isRunning)
    {
        clock_gettime(CLOCK_MONOTONIC, &this->startAt);
        // incrementTimeSpec(&this->startAt, startTimeMicros*1000);
        this->interval_ns = intervalMicros*1000;
        this->reportedOffset = startTimeMicros;

        // this->startTime = startTimeMicros;
        // this->endTime = 0;
        // this->interval = intervalMicros;
        this->tickCallback = tickCallback;
        this->stopCallback = stopCallback;

        this->microTime = 0;

        this->isRunning = true;
        this->timerThread = new std::thread(&Timer::internalContinousLoop, this);
        this->timerThread->detach();
    }
    else
    {
        Debug::error("timer already running");
    }
}

void Timer::stop()
{
    if (this->isRunning)
    {
        this->isRunning = false;
        syncMtx.lock();
        std::thread callbackThread(this->stopCallback);
        callbackThread.detach();
        delete this->timerThread;
        syncMtx.unlock();
    }
}

void Timer::tick(Timer* self, uint64 interval, int64 endTime, int64 microTime)
{
    std::unique_lock<std::mutex> lock(self->syncMtx);
    while(self->isRunning) {

        std::thread callbackThread(self->tickCallback, microTime);
        callbackThread.detach();

        usleep(interval);

        if (microTime % 500000 == 0)
        {
            Debug::print("Micro Seconds: %d", microTime);
            std::cout << microTime << std::endl;
        }
        microTime += interval;


        if (microTime >= endTime)
        {
            lock.unlock();
            self->stop();
        }
    }
}

/**
    DO NOT INCREMENT MORE THAN 1SECOND AT TIME
*/
void Timer::incrementTimeSpec(struct timespec *ts, long nsec){
    ts->tv_nsec += nsec;
    normalizeTimestamp(ts);
}

/**
    Only fixes timespec only when the nanoseconds exceed a second
*/
void Timer::normalizeTimestamp(struct timespec *ts){
    long sec = ts->tv_nsec / NSEC_PER_SEC;
    ts->tv_nsec = ts->tv_nsec % NSEC_PER_SEC;
    ts->tv_sec += sec;
}

/**
 * High precision timer that uses the monotonic clock as source, interval should
 * not be bigger than 1 second
 *   DO NOT PROVIDE A TIMESPEC THAT IS SMALLER THAN CURRENT TIME
 *
 * @param interval_nsec   interval in nanoseconds (must not be more than 1 second)
 * @param startAt         time to start the execution (must not be a time of the past)
*/
void Timer::internalContinousLoop(void){

    struct timespec next_expiration;
    clock_gettime(CLOCK_MONOTONIC, &next_expiration);

    while(isRunning){
        tickCallback(TS_TO_MILLI(next_expiration)-reportedOffset);
        incrementTimeSpec(&next_expiration, interval_ns);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_expiration, NULL);
    }
}

void Timer::internalLoop(void){

    struct timespec next_expiration;
    clock_gettime(CLOCK_MONOTONIC, &next_expiration);
    memcpy(&startAt, &next_expiration, sizeof(struct timespec));
    reportedOffset = TS_TO_MILLI(next_expiration) - reportedOffset;

    while(isRunning){
        int64 visual_time = TS_TO_MILLI(next_expiration)- reportedOffset;
        tickCallback(visual_time);
        incrementTimeSpec(&next_expiration, interval_ns);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_expiration, NULL);
        if(visual_time >= endTime){
            stop();
        }
    }
}

void Timer::highPerformanceTimerLoop(Timer* self, uint64 interval, int64 endTime, int64 microTime)
{
    std::unique_lock<std::mutex> lock(self->syncMtx);
    auto lastTime = Clock::now();
    auto currTime = lastTime;
    std::chrono::microseconds stepTime(interval);
    uint64 currCheckTime = 0;

    std::thread callbackThread(self->tickCallback, microTime);
    callbackThread.detach();

    while(self->isRunning) {

        currTime = Clock::now();
        currCheckTime = std::chrono::duration_cast<std::chrono::microseconds>(currTime-lastTime).count();

        if (currCheckTime >= interval)
        {
            microTime += interval;

            std::thread callbackThread(self->tickCallback, microTime);
            callbackThread.detach();
	        //self->tickCallback(microTime);
            lastTime = lastTime + stepTime;

            if (microTime >= endTime)
            {
                lock.unlock();
                self->stop();
            }
        }

        usleep(100);
    }
}

void Timer::highPerformanceContinousTimerLoop(Timer* self, uint64 interval, int64 microTime)
{
    std::unique_lock<std::mutex> lock(self->syncMtx);
    auto lastTime = Clock::now();
    auto currTime = lastTime;
    std::chrono::microseconds stepTime(interval);
    int64 currCheckTime = 0;

    std::thread callbackThread(self->tickCallback, microTime);
    callbackThread.detach();

    while(self->isRunning) {

        currTime = Clock::now();
        currCheckTime = std::chrono::duration_cast<std::chrono::microseconds>(currTime-lastTime).count();

        if (currCheckTime >= interval)
        {
            microTime += interval;

            std::thread callbackThread(self->tickCallback, microTime);
            callbackThread.detach();
            //self->tickCallback(microTime);
            lastTime = lastTime + stepTime;
        }
        //std::this_thread::sleep_for(std::chrono::microseconds(100));
        usleep(100);
    }
}

