#pragma once
/*
 * SyncUnit.h
 *
 *  Created on: Dec 22, 2015
 *      Author: tzhou
 */
#ifndef DRIVER_TOOLS_SYNCUNIT_H_
#define DRIVER_TOOLS_SYNCUNIT_H_

#include <condition_variable>


/*
 * A Synchronization unit which focus on the shared variable's state.
 * wait() -> wait for it to be ready
 * notify() -> set it to be ready, at the same time wake up waiters
 * reset() -> set it to be not ready
 * NOTE:
 * It is allowed to notify() before wait(). This will let wait() return directly.
 */
struct SyncUnit{
	void wait(){
		if(ready)	return;
		std::unique_lock<std::mutex> ul(m);
		if(ready)	return;
		cv.wait(ul,[&](){return ready;});
	}
	bool wait_for(const std::chrono::duration<double>& dur){
		if(ready)	return true;
		std::unique_lock<std::mutex> ul(m);
		if(ready)	return true;
		return cv.wait_for(ul,dur,[&](){return ready;});
	}
	template<class rep, class period>
	bool wait_for(const std::chrono::duration<rep,period>& dur){
		return wait_for(std::chrono::duration_cast<double>(dur));
	}
	void notify(){
		ready=true;
		cv.notify_all();
	}
	void reset(){
		ready=false;
	}
private:
	std::mutex m;
	std::condition_variable cv;
	bool ready=false;
};

#endif /* DRIVER_TOOLS_SYNCUNIT_H_ */
