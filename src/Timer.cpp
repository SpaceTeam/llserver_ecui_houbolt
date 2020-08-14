//
// Created by Markus on 2019-09-27.
//


#include "Timer.h"

#include <chrono>
#include <iostream>
// #include <boost/asio.hpp>
// #include <boost/date_time/posix_time/posix_time.hpp>

//void tickFunc(std::function<void()> stopCallback, std::function<void()> stopCallback, uint64 interval, int64 endTime, int64 microTime)
//{
//    std::cout << interval << std::endl;
//    bool running = true;
//    while(running) {
//
//        usleep(interval);
//
//        std::thread callbackThread(self->tickCallback, self->microTime);
//        callbackThread.detach();
//        if (self->microTime % 500000 == 0)
//        {
//            Debug::print("Micro Seconds: %d", self->microTime);
//            std::cout << self->microTime << std::endl;
//        }
//        microTime += interval;
//
//        if (microTime >= endTime)
//        {
//            std::cout << "sequence done" << std::endl;
//            self->stop();
//            running = false;
//        }
//    }
//}

typedef std::chrono::high_resolution_clock Clock;

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

        this->startTime = startTimeMicros;
        this->endTime = endTimeMicros;
        this->interval = intervalMicros;
        this->tickCallback = tickCallback;
        this->stopCallback = stopCallback;

        this->microTime = startTimeMicros;

        this->isRunning = true;
        this->timerThread = new std::thread(Timer::highPerformanceTimerLoop, this, intervalMicros, endTimeMicros, startTimeMicros);
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

        this->startTime = startTimeMicros;
        this->endTime = 0;
        this->interval = intervalMicros;
        this->tickCallback = tickCallback;
        this->stopCallback = stopCallback;

        this->microTime = 0;

        this->isRunning = true;
        this->timerThread = new std::thread(Timer::highPerformanceContinousTimerLoop, this, intervalMicros, startTimeMicros);
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

void Timer::highPerformanceTimerLoop(Timer* self, uint64 interval, int64 endTime, int64 microTime)
{
    std::unique_lock<std::mutex> lock(self->syncMtx);
    auto lastTime = Clock::now();
    auto currTime = lastTime;
    uint64 currCheckTime = 0;

    std::thread callbackThread(self->tickCallback, microTime);
    callbackThread.detach();

    // boost::asio::io_service io;
    // // Construct a timer without setting an expiry time.
    // boost::asio::deadline_timer timer(io);
    while(self->isRunning) {

        currTime = Clock::now();
        if (std::chrono::duration_cast<std::chrono::microseconds>(currTime-lastTime).count() >= interval)
        {
            microTime += interval;

            std::thread callbackThread(self->tickCallback, microTime);
            callbackThread.detach();
	    //self->tickCallback(microTime);
            lastTime = currTime;

//            if (microTime % 500000 == 0)
//            {
//                std::cout << "Curr Interval: " << std::chrono::duration_cast<std::chrono::microseconds>(currTime-lastTime).count() << std::endl;
//            }

            if (microTime >= endTime)
            {
                lock.unlock();
                self->stop();
            }
        }

        usleep(100);

//        // Set an expiry time relative to now.
//        timer.expires_from_now(boost::posix_time::microseconds(interval));
//
//        // Wait for the timer to expire.
//        timer.wait();
//
//        currTime = Clock::now();
//        currCheckTime = std::chrono::duration_cast<std::chrono::microseconds>(currTime-lastTime).count();
//        //Debug::error("%d", currTime-lastTime);
//        if (currCheckTime > interval + 1000)
//        {
//            std::cout << currCheckTime << std::endl;
//        }
//
//        microTime += interval;
//
//        std::thread callbackThread(self->tickCallback, microTime);
//        callbackThread.detach();
//        //self->tickCallback(microTime);
//        lastTime = currTime;
//
//        if (microTime >= endTime)
//        {
//            lock.unlock();
//            self->stop();
//        }
    }
}

void Timer::highPerformanceContinousTimerLoop(Timer* self, uint64 interval, int64 microTime)
{
    std::unique_lock<std::mutex> lock(self->syncMtx);
    auto lastTime = Clock::now();
    auto currTime = lastTime;
    int64 currCheckTime = 0;

    std::thread callbackThread(self->tickCallback, microTime);
    callbackThread.detach();

    // boost::asio::io_service io;
    // // Construct a timer without setting an expiry time.
    // boost::asio::deadline_timer timer(io);

    while(self->isRunning) {

        currTime = Clock::now();
        currCheckTime = std::chrono::duration_cast<std::chrono::microseconds>(currTime-lastTime).count();

        if (currCheckTime > interval + 1000)
        {
            std::cout << currCheckTime << std::endl;
        }

        if (currCheckTime >= interval)
        {
            microTime += interval;

            std::thread callbackThread(self->tickCallback, microTime);
            callbackThread.detach();
            //self->tickCallback(microTime);
            lastTime = currTime;
        }
        //std::this_thread::sleep_for(std::chrono::microseconds(100));
        usleep(100);

//        // Set an expiry time relative to now.
//        timer.expires_from_now(boost::posix_time::microseconds(interval));
//
//        // Wait for the timer to expire.
//        timer.wait();
//
//        currTime = Clock::now();
//        currCheckTime = std::chrono::duration_cast<std::chrono::microseconds>(currTime-lastTime).count();
//        //Debug::error("%d", currTime-lastTime);
//        if (currCheckTime > interval + 1000)
//        {
//            std::cout << currCheckTime << std::endl;
//        }
//
//        microTime += interval;
//
//        std::thread callbackThread(self->tickCallback, microTime);
//        callbackThread.detach();
//        //self->tickCallback(microTime);
//        lastTime = currTime;
    }
}

