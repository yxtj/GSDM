#pragma once
#include <chrono>
class Timer
{
	std::chrono::system_clock::time_point _time;
public:
	Timer();
	void restart();

	// return millisecond as default
	long long elapse() const {
		return elapseMS();
	}
	long long elapseMS() const;
	long long elapseS() const;
	double elapseMin() const;
};

