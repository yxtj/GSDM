#pragma once
/*
 * Dispatcher.h
 *
 *  Created on: Dec 15, 2015
 *      Author: tzhou
 */
#ifndef DRIVER_DISPATCHER_HPP_
#define DRIVER_DISPATCHER_HPP_

#include <unordered_map>
#include <functional>
#include <condition_variable>
#include <functional>

template<class... Params>
class Dispatcher{
public:
	typedef std::function<void(Params...)> callback_t;

	Dispatcher(int initTypeNum=50, float loadFactor=0.8f):
		workLoad(0),callbacks_(InitTypeNum),paused_(false){
		callbacks_.max_load_factor(loadFactor);
	}

	void registerDispFun(const int type, callback_t cb);
	void unregisterDispFun(const int type);
	void clear();

	void pause();
	void resume();

	bool canHandle(const int type) const;
	bool busy() const{	//is there any data being dispatched now
		return workLoad!=0;
	}

	bool receiveData(const int type, Params... param) const;
	void runWithData(const int type, Params... param) const;
private:
	mutable int workLoad;
	std::unordered_map<int, callback_t> callbacks_;
//	std::pair<callback_t, bool> callbacks_[100];

	bool paused_;
	mutable std::mutex m;
	mutable std::condition_variable cv;

	inline void launch(const callback_t& target, Params... param) const;
};

template<class... Params>
void Dispatcher<Params...>::registerDispFun(const int type, callback_t cb){
	std::lock_guard<decltype(m)> l(m);
	callbacks_[type]=cb;
}

template<class... Params>
void Dispatcher<Params...>::unregisterDispFun(const int type){
	std::lock_guard<decltype(m)> l(m);
	callbacks_.erase(type);
//	callbacks_[type].second=false;
}

template<class... Params>
void Dispatcher<Params...>::clear(){
	std::lock_guard<decltype(m)> l(m);
	callbacks_.clear();
}

template<class... Params>
void Dispatcher<Params...>::pause(){
	paused_=true;
}
template<class... Params>
void Dispatcher<Params...>::resume(){
	paused_=false;
	cv.notify_all();
}


template<class... Params>
bool Dispatcher<Params...>::canHandle(const int type) const{
	return callbacks_.find(type)!=callbacks_.end();
}

template<class... Params>
bool Dispatcher<Params...>::receiveData(int type, Params... param) const{
	++workLoad;
	bool ret=false;
	auto it=callbacks_.find(type);
	if(it!=callbacks_.end()){
		launch(it->second, param...);
		ret=true;
	}
	--workLoad;
	return ret;
}

template<class... Params>
void Dispatcher<Params...>::runWithData(int type, Params... param) const{
	++workLoad;
	launch(callbacks_.at(type),param...);
	--workLoad;
}

template<class... Params>
void Dispatcher<Params...>::launch(
		const callback_t& target, Params... param) const{
	if(paused_){
		std::unique_lock<decltype(m)> ul(m);
		cv.wait(ul, [&]{ return !paused_;});
	}
	target(param...);
}

#endif /* DRIVER_DISPATCHER_HPP_ */
