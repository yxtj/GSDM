/*
 * MsgDriver.cpp
 *
 *  Created on: Dec 15, 2015
 *      Author: tzhou
 */
#include "MsgDriver.h"

using namespace std;

//Helper
MsgDriver::callback_t MsgDriver::GetDummyHandler(){
	static callback_t dummy=[](const std::string&, const RPCInfo&){};
	return dummy;
}
MsgDriver::MsgDriver()
{
	clear();
}

bool MsgDriver::empty() const{
	return !inDisper.busy() && que.empty();
}
bool MsgDriver::emptyQueue() const{
	return que.empty();
}
bool MsgDriver::busy() const{
	return inDisper.busy() || !outDisper.busy() || que.empty();
}

//Register
void MsgDriver::registerImmediateHandler(const int type, callback_t cb){
	inDisper.registerDispFun(type,cb);
}
void MsgDriver::unregisterImmediateHandler(const int type){
	inDisper.unregisterDispFun(type);
}
void MsgDriver::registerProcessHandler(const int type, callback_t cb){
	outDisper.registerDispFun(type,cb);
}
void MsgDriver::unregisterProcessHandler(const int type){
	outDisper.unregisterDispFun(type);
}
void MsgDriver::registerDefaultOutHandler(callback_t cb){
	defaultHandler=cb;
}

//reset & clear
void MsgDriver::resetImmediateHandler(){
	inDisper.clear();
}
void MsgDriver::resetProcessHandler(){
	outDisper.clear();
}
void MsgDriver::resetDefaultOutHandler(){
	defaultHandler=GetDummyHandler();
}
void MsgDriver::resetWaitingQueue(){
	lock_guard<mutex> ql(queLock);
	que.clear();
}
void MsgDriver::clear(){
	resetImmediateHandler();
	resetProcessHandler();
	resetWaitingQueue();
	resetDefaultOutHandler();
}
size_t MsgDriver::abandonData(const int type){
	lock_guard<mutex> ql(queLock);
	size_t f=0,l=0;
	while(l<que.size()){
		if(que[l].second.tag==type){
			que[f++]=move(que[l++]);
		}else{
			++l;
		}
	}
	que.erase(que.begin()+f,que.end());
	return l-f;
}

//Main working functions
bool MsgDriver::pushData(string& data, const RPCInfo& info) {
	return processInput(move(data), info);
}
bool MsgDriver::pushData(string&& data, const RPCInfo& info) {
	return processInput(move(data), info);
}
bool MsgDriver::popData() {
	if(que.empty())	return false;
	string d;
	RPCInfo r;
	{
		lock_guard<mutex> ql(queLock);
		tie(d,r)=move(que.front());
		que.pop_front();
	}
	return processOutput(d, r);
//	auto p = move(que.front());
//	que.pop_front();
//	return processOutput(p.first, p.second);
}

//Process
bool MsgDriver::processInput(string&& data, const RPCInfo& info){
	if(!inDisper.receiveData(info.tag, data, info)){
		{
			lock_guard<mutex> ql(queLock);
			que.push_back(make_pair(move(data), move(info)));
		}
		return true;
	}
	return false;
}
bool MsgDriver::processOutput(string& data, const RPCInfo& info){
	if(!outDisper.receiveData(info.tag, data, info)){
		defaultHandler(data,info);
		return true;
	}
	return false;
}
