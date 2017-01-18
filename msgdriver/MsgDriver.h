#pragma once
/*
 * MsgDriver.h
 *
 *  Created on: Dec 15, 2015
 *      Author: tzhou
 */

#ifndef DRIVER_MSGDRIVER_H_
#define DRIVER_MSGDRIVER_H_

#include "Dispatcher.hpp"
#include "../netbase/RPCInfo.h"
#include <deque>
#include <string>
#include <mutex>
#include <functional>

/*
 * The data flow is as below:
 * data->immediateDispatcher-+-> queue-> processDispatcher-+-> end
 *                           +-> processed-> end           +-> defaultHandle-> end
 */
class MsgDriver{
public:
	typedef Dispatcher<const std::string&, const RPCInfo&>::callback_t callback_t;
	static callback_t GetDummyHandler();

	MsgDriver();
	void terminate();
	bool empty() const;
	bool emptyStrict() const;
	bool busy() const;


	// For message should be handled at receiving time (i.e. alive check)
	void registerImmediateHandler(const int type, callback_t cb);
	void unregisterImmediateHandler(const int type);
	// For message should be handled in sequence (i.e. data update)
	void registerProcessHandler(const int type, callback_t cb);
	void unregisterProcessHandler(const int type);

	void registerDefaultOutHandler(callback_t cb);

	void resetImmediateHandler();
	void resetProcessHandler();
	void resetDefaultOutHandler();
	void resetWaitingQueue();
	void clear();

	size_t abandonData(const int type);

	const std::deque<std::pair<std::string, RPCInfo> >& getQue() const{
		return que;
	}
	size_t queSize() const {return que.size();}

	const std::pair<std::string, RPCInfo>& front() const{
		return que.front();
	}
	const std::pair<std::string, RPCInfo>& back() const{
		return que.back();
	}

	// return whether the input bypasses the dispatcher (enqueue)
	bool pushData(std::string& data, RPCInfo& info);
	bool pushData(std::string&& data, RPCInfo& info);
	// return whether a value is picked and bypasses the dispatcher (default handled)
	bool popData();

private:
	//return whether the provided data bypasses dispatcher (enqueue & default handled)
	bool processInput(std::string&& data, RPCInfo& info);
	bool processOutput(std::string& data, RPCInfo& info);

	bool running_;

	Dispatcher<const std::string&, const RPCInfo&> inDisper; //immediately response
	std::deque<std::pair<std::string, RPCInfo> > que; //queue for message waiting for process
//	mutable std::mutex lockQue;
	Dispatcher<const std::string&, const RPCInfo&> outDisper; //response when processed
	callback_t defaultHandler;
};

#endif /* DRIVER_MSGDRIVER_H_ */
