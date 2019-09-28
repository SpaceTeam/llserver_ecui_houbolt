//
// Created by Markus on 2019-09-27.
//

#include "Timer.h"

Timer::Timer()
{

    this->microTime = 0;

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
        this->timerThread = new std::thread(Timer::tick, this);

        this->timerThread->detach();
    }
    else
    {
        Debug::error("timer already running");
    }
}

void Timer::stop()
{
    this->stopCallback();
    this->isRunning = false;
}

void Timer::tick(Timer* self)
{
    while(self->isRunning) {

        std::this_thread::sleep_for(std::chrono::microseconds(self->interval));

        self->tickCallback(self->microTime);
        self->microTime += self->interval;

        if (self->microTime >= self->endTime)
        {
            self->stop();
        }
    }
}