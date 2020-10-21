//
// Created by Markus on 2019-09-27.
//


#include "Timer.h"

#include <chrono>
#include <iostream>
#include <algorithm>
#include <sys/time.h>

#define TS_TO_MICRO(x) (int64)(((int64)(x.tv_nsec) / 1000)+((int64)(x.tv_sec)*1000000))

int Timer::nameIndex = 0;

Timer::Timer(int prio, std::string name)
{
    
    threadName = name;
    microTime = 0;
    threadPriority = prio;

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
        isRunning = false;
        syncMtx.lock();
        stopCallback();
        delete this->timerThread;
        syncMtx.unlock();
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

    struct sched_param param;
    param.sched_priority = threadPriority;;
    sched_setscheduler(0, SCHED_FIFO, &param);

//    cpu_set_t my_set;        /* Define your cpu_set bit mask. */
//    CPU_ZERO(&my_set);       /* Initialize it all to 0, i.e. no CPUs selected. */
//    CPU_SET(3, &my_set);     /* set the bit that represents core 7. */
//    sched_setaffinity(0, sizeof(cpu_set_t), &my_set);

    struct timespec next_expiration;
    clock_gettime(CLOCK_MONOTONIC, &next_expiration);
    reportedOffset = TS_TO_MICRO(next_expiration) - reportedOffset;

#ifdef ENABLE_TIMER_DIAGNOSTICS
    int counts = 0;
    int64 lastExceed = TS_TO_MICRO(next_expiration);
#endif

    while(isRunning){

#ifdef ENABLE_TIMER_DIAGNOSTICS
        struct timespec start;
        clock_gettime(CLOCK_MONOTONIC, &start);
#endif

        /** Sequence Time is the used in rocket launches (where 0 is the ignition) */
        int64 sequence_time = TS_TO_MICRO(next_expiration)- reportedOffset;
        tickCallback(sequence_time);

#ifdef ENABLE_TIMER_DIAGNOSTICS
        struct timespec end;
        clock_gettime(CLOCK_MONOTONIC, &end);

        long curr_exec = TS_TO_MICRO(end) - TS_TO_MICRO(start);
        long diff = TS_TO_MICRO(start)-TS_TO_MICRO(next_expiration);
        printf("%s - exec_time: %ld\n", threadName.c_str(), curr_exec);
        if(curr_exec > max_exec_time){
            max_exec_time = curr_exec;
        }
        loops_cnt++;

        if(loops_cnt > report_loops){
            printf("%s - max_exec: %06ld\n", threadName.c_str(), max_exec_time);
            loops_cnt=0;
        }

        if (diff > 30){
            // printf("TS: %ld, %09ld, SEQTIME: %ld\n", start.tv_sec, start.tv_nsec, sequence_time);
            printf("%s - offset: %04ld, ticks_ok: %d, exec_time: %ld\n", threadName.c_str(), diff, counts, curr_exec);
            //printf("trueTimeMircros: %lld, nextExp: %lld, repOffset: %lld\n", TS_TO_MICRO(start) - reportedOffset,  TS_TO_MICRO(next_expiration), reportedOffset);
            counts = 0;
            lastExceed = TS_TO_MICRO(next_expiration);
        }else{
            counts++;
        }
#endif

        incrementTimeSpec(&next_expiration, interval_ns);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_expiration, NULL);
    }

#ifdef ENABLE_TIMER_DIAGNOSTICS
    printf("Threat Return\n");
#endif
}

void Timer::internalLoop(void){

    struct timespec next_expiration;
    clock_gettime(CLOCK_MONOTONIC, &next_expiration);
    reportedOffset = TS_TO_MICRO(next_expiration) - reportedOffset;

    while(isRunning){
        /** Sequence Time is the used in rocket launches (where 0 is the ignition) */
        int64 sequence_time = TS_TO_MICRO(next_expiration)- reportedOffset;
//        printf("SEQTIME: %lld\n", sequence_time);

        tickCallback(sequence_time);
        incrementTimeSpec(&next_expiration, interval_ns);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_expiration, NULL);
        if(sequence_time >= endTime){
            stop();
        }
    }

#ifdef ENABLE_TIMER_DIAGNOSTICS
    printf("Threat Return\n");
#endif
}
