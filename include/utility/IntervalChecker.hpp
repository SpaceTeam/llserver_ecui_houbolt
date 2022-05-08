#pragma once

#include <chrono>
#include <string>


class IntervalChecker
{
	public:
		IntervalChecker(uint32_t _maxInterval_us, std::string _name);
		~IntervalChecker() {};
		int check();

	private:
		uint32_t maxInterval_us;
		std::string name;

		std::chrono::steady_clock::time_point lastTime;
};
