#pragma once
/*
 * SyncUnit.h
 *
 *  Created on: Dec 22, 2015
 *  Modified on Jan 18, 2017
 *      Author: tzhou
 */
#ifndef DRIVER_TOOLS_SYNCUNIT_H_
#define DRIVER_TOOLS_SYNCUNIT_H_

#include <condition_variable>


/*
 * An wrapped unit used to synchronize across multiple threads.
 * It checks & manipulates the state of a shared variable.
 * API:
 *   wait() -> wait until the state is ready
 *   wait_for() -> wait for the state to be ready for at most a given time.
 *   notify() -> set the state to be ready, at the same time wake up all waiters
 *   reset() -> reset the state to be not ready
 * NOTE:
 *   It is allowed to notify() before wait().
 *   This will make wait() and wait_for() return directly.
 */
struct SyncUnit{
	// wait until the state is ready.
	void wait();
	// wait for at most $dur$ seconds.
	// return whether the state is ready.
	//   true -> waken up by notification
	//   false -> waken up by timeout
	bool wait_for(const double& dur);

	template<class rep, class period>
	bool wait_for(const std::chrono::duration<rep,period>& dur) {
		if(ready)	return true;
		std::unique_lock<std::mutex> ul(m);
		if(ready)	return true;
		return cv.wait_for(ul, dur, [&]() {return ready; });
	}
	// wake up all waiters
	void notify();

	void reset(){
		ready=false;
	}
private:
	std::mutex m;
	std::condition_variable cv;
	bool ready=false;
};

#endif /* DRIVER_TOOLS_SYNCUNIT_H_ */
