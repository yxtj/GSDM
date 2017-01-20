/*
* SyncUnit.cpp
*
*  Created on: Jan 19, 2017
*      Author: tzhou
*/
#include "SyncUnit.h"

void SyncUnit::wait() {
	if(ready)	return;
	std::unique_lock<std::mutex> ul(m);
	if(ready)	return;
	cv.wait(ul, [&]() {return ready; });
}

bool SyncUnit::wait_for(const double& dur) {
	return wait_for(std::chrono::duration<double>(dur));
}

void SyncUnit::notify() {
	ready = true;
	cv.notify_all();
}
