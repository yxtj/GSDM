#include "Timer.h"

using namespace std;

Timer::Timer()
{
	restart();
}

void Timer::restart()
{
	_time = chrono::system_clock::now();
}

long long Timer::elapseMS() const
{
	return chrono::duration_cast<chrono::milliseconds>(
		chrono::system_clock::now() - _time).count();
}

long long Timer::elapseS() const
{
	return chrono::duration_cast<chrono::seconds>(
		chrono::system_clock::now() - _time).count();
}

double Timer::elapseMin() const
{
	std::chrono::duration<double, ratio<60> > passed = chrono::system_clock::now() - _time;
	return passed.count();
}

