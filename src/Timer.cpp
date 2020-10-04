//
// Created by Markus on 2019-09-27.
//


#include "Timer.h"

#include <chrono>
#include <iostream>
#include <sys/time.h>

typedef std::chrono::high_resolution_clock Clock;

#define TS_TO_MILLI(x) (int64)(((int64)(x.tv_nsec) / 1000)+((int64)(x.tv_sec)*1000000))

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

void Timer::startContinous(int64 startTimeMicros, uint64 intervalMicros, std::function<void(int64)> tickCallback, std::function<void()> stopCallback)
{
    if (!this->isRunning)
    {
        this->interval_ns = intervalMicros*1000;
        this->reportedOffset = startTimeMicros;

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
    //TODO: Fixup cleaning
    if (isRunning)
    {
        printf("Thread Stopping\n");
        isRunning = false;
        syncMtx.lock();
        stopCallback();
        delete this->timerThread;
        syncMtx.unlock();
        printf("Thread Finished\n");
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
    reportedOffset = TS_TO_MILLI(next_expiration) - reportedOffset;
	int counts = 0;
	int64 lastExceed = TS_TO_MILLI(next_expiration);
    while(isRunning){
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        /** Sequence Time is the used in rocket launches (where 0 is the ignition) */
        int64 sequence_time = TS_TO_MILLI(next_expiration)- reportedOffset;
if (TS_TO_MILLI(now)-TS_TO_MILLI(next_expiration) > 30){
        printf("TS: %ld, %09ld, SEQTIME: %lld\n", now.tv_sec, now.tv_nsec, sequence_time);
	printf("offset: %lld, ticks_since_last_exceed: %d, time_last_exc: %lld\n", TS_TO_MILLI(now)-TS_TO_MILLI(next_expiration), counts, TS_TO_MILLI(next_expiration) - lastExceed);
//printf("trueTimeMircros: %lld, nextExp: %lld, repOffset: %lld\n", TS_TO_MILLI(now) - reportedOffset,  TS_TO_MILLI(next_expiration), reportedOffset);
counts = 0;
lastExceed = TS_TO_MILLI(next_expiration);
}else{counts++;}
//tickCallback(sequence_time);
std::thread callbackThread(tickCallback, sequence_time);
            callbackThread.detach();
        incrementTimeSpec(&next_expiration, interval_ns);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_expiration, NULL);
    }

    printf("Threat Return\n");
}

void Timer::internalLoop(void){

    struct timespec next_expiration;
    clock_gettime(CLOCK_MONOTONIC, &next_expiration);
    reportedOffset = TS_TO_MILLI(next_expiration) - reportedOffset;

    while(isRunning){
        /** Sequence Time is the used in rocket launches (where 0 is the ignition) */
        int64 sequence_time = TS_TO_MILLI(next_expiration)- reportedOffset;
//        printf("SEQTIME: %lld\n", sequence_time);

        tickCallback(sequence_time);
        incrementTimeSpec(&next_expiration, interval_ns);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_expiration, NULL);
        if(sequence_time >= endTime){
            stop();
        }
    }

    printf("Threat Return\n");
}
