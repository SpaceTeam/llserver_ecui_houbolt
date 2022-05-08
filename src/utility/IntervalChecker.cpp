#include "utility/IntervalChecker.hpp"
#include "utility/Debug.h"


IntervalChecker::IntervalChecker(uint32_t _maxInterval_us, std::string _name)
{
	maxInterval_us = _maxInterval_us;
	name = _name;
	lastTime = std::chrono::steady_clock::now();
}

int IntervalChecker::check()
{
	auto time = std::chrono::steady_clock::now();
	uint32_t interval_us = std::chrono::duration_cast<std::chrono::microseconds>(time - lastTime).count();
	lastTime = time;
	if(interval_us > maxInterval_us)
	{
		Debug::warning(name + " interval high: %uµs, limit: %uµs", interval_us, maxInterval_us);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
