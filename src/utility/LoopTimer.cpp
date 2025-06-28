#include "utility/LoopTimer.hpp"

#include "utility/Debug.h"


LoopTimer::LoopTimer(uint32_t _interval_us, std::string _name, float tolerance)
{
	interval_us = _interval_us;
	maxInterval_us = (uint32_t)((float)_interval_us * (1.0 + tolerance) + 0.5);
	name = _name;
	startTime = {};
	time = {};
	nextTime = {};
	lastTime = {};
}

void LoopTimer::init()
{
	startTime = std::chrono::steady_clock::now();
	lastTime = startTime;
	nextTime = startTime;
	time = startTime;
}

int LoopTimer::wait()
{
	// wait till next interval passed
	nextTime += std::chrono::microseconds(interval_us);
	std::this_thread::sleep_until(nextTime);

	// check timing
	time = std::chrono::steady_clock::now();
	uint32_t interval_us = std::chrono::duration_cast<std::chrono::microseconds>(time - lastTime).count();
	lastTime = time;
	if(interval_us > maxInterval_us)
	{
		Debug::warning(name + " interval high: %uµs, limit: %uµs", interval_us, maxInterval_us);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

uint64_t LoopTimer::getTimePoint_us() const {
	return std::chrono::time_point_cast<std::chrono::microseconds>(time).time_since_epoch().count();
}

uint64_t LoopTimer::getTimeElapsed_us() const {
	return std::chrono::duration_cast<std::chrono::microseconds>(time - startTime).count();
}
