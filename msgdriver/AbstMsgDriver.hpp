#pragma once
/*
* AbstMsgDriver.hpp
*
*  Created on: Dec 15, 2015
*      Author: tzhou
*  Motified on: Aug 30, 2016
*/
#ifndef DRIVER_MSGDRIVER_H_
#define DRIVER_MSGDRIVER_H_

#include "Dispatcher.hpp"
#include <deque>
#include <string>
#include <mutex>
#include <functional>

/*
 * The data flow is as below:
 * data->immediateDispatcher-+-> queue-> processDispatcher-+-> end
 *                           +-> processed-> end           +-> defaultHandle-> end
 */
template<class... Params>
class AbstMsgDriver{
public:
	typedef Dispatcher<std::add_lvalue_reference<Params>...>::callback_t callback_t;
	static callback_t GetDummyHandler() {
		static callback_t dummy = [](std::add_lvalue_reference<Params>...) {};
		return dummy;
	}

	MsgDriver() : running_(false) {
		clear();
	}
	void terminate() {
		running_ = false;
	}
	bool empty() const {
		return que.empty();
	}
	bool emptyStrict() const {
		return !inDisper.busy() && que.empty();
	}
	bool busy() const {
		return inDisper.busy() || !outDisper.busy() || que.empty();
	}


	// For message should be handled at receiving time (i.e. alive check)
	void registerImmediateHandler(const int type, callback_t cb) {
		inDisper.registerDispFun(type, cb);
	}
	void unregisterImmediateHandler(const int type) {
		inDisper.unregisterDispFun(type);
	}
	// For message should be handled in sequence (i.e. data update)
	void registerProcessHandler(const int type, callback_t cb) {
		outDisper.registerDispFun(type, cb);
	}
	void unregisterProcessHandler(const int type) {
		outDisper.unregisterDispFun(type);
	}

	void registerDefaultOutHandler(callback_t cb) {
		defaultHandler = cb;
	}

	void resetImmediateHandler() {
		inDisper.clear();
	}
	void resetProcessHandler() {
		outDisper.clear();
	}
	void resetDefaultOutHandler() {
		defaultHandler = GetDummyHandler();
	}
	void resetWaitingQueue() {
		//	lock_guard<mutex> ql(lockQue);
		que.clear();
	}
	void clear() {
		resetImmediateHandler();
		resetProcessHandler();
		resetWaitingQueue();
		resetDefaultOutHandler();
	}

	size_t abandonData(const int type) {
		//	lock_guard<mutex> ql(lockQue);
		size_t f = 0, l = 0;
		while(l<que.size()) {
			if(que[l].second.tag == type) {
				que[f++] = move(que[l++]);
			} else {
				++l;
			}
		}
		que.erase(que.begin() + f, que.end());
		return l - f;
	}

	const std::deque<std::tuple<Params...> >& getQue() const{
		return que;
	}
	size_t queSize() const {return que.size();}

	const std::tuple<Params...>& front() const {
		return que.front();
	}
	const std::tuple<Params...>& back() const{
		return que.back();
	}

	// return whether the input bypasses the dispatcher (enqueue)
	bool pushData(int type, std::forward<Params>... params) {
		return processInput(std::forward(params)...);
	}
	// return whether a value is picked and bypasses the dispatcher (default handled)
	bool popData() {
		if(que.empty())	return false;
		auto p = move(que.front());
		que.pop_front();
		return processOutput(std::forward(p)...);
	}

private:
	//return whether the provided data bypasses dispatcher (enqueue & default handled)
	bool processInput(int type, std::forward<Params>... params) {
		if(!inDisper.receiveData(type, std::forward(params)...)) {
			//		lock_guard<mutex> ql(lockQue);
			que.push_back(std::forward(params)...);
			return true;
		}
		return false;
	}

	bool running_;

	Dispatcher<std::add_lvalue_reference<Params>...> inDisper; //immediately response
	std::deque<std::tuple<Params...> > que; //queue for message waiting for process
//	mutable std::mutex lockQue;
	Dispatcher<std::add_lvalue_reference<Params>...> outDisper; //response when processed
	callback_t defaultHandler;
};

#endif /* DRIVER_MSGDRIVER_H_ */
