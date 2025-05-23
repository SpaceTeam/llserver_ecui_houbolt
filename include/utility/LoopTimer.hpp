#pragma once

#include <chrono>
#include <string>


class LoopTimer
{
	public:
		LoopTimer(uint32_t _interval_us, std::string _name, float tolerance = 0.1);
		~LoopTimer() {};

		void init();
		int wait();
		uint64_t getTimePoint_us() const;
		uint64_t getTimeElapsed_us() const;

	private:
		uint32_t interval_us;
		uint32_t maxInterval_us;
		std::string name;

		std::chrono::steady_clock::time_point startTime;
		std::chrono::steady_clock::time_point time;
		std::chrono::steady_clock::time_point nextTime;
		std::chrono::steady_clock::time_point lastTime;
};
