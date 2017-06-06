/*
 * ReplyHandler.cpp
 *
 *  Created on: Dec 20, 2015
 *      Author: tzhou
 */

#include "ReplyHandler.h"
#include <algorithm>
#include <thread>
#include <mutex>

using namespace std;

/*
 * Condition part:
 */

struct ConditionAny:public ReplyHandler::Condition{
	bool update(const int source){ return true; }
};
struct ConditionEachOne:public ReplyHandler::Condition{
	ConditionEachOne(const int num):state(num,false){}
	bool update(const int source){
		// writing on vector<bool> is not thread safe. (8-bit in one byte)
		// when multiple thread try to write the bits in the same byte, the new one over-writing old ones.
		lock_guard<mutex> lg(ms);
		state.at(source) = true;
		if(all_of(state.begin(), state.end(), [](const bool b){return b;})){
			return true;
		}
		return false;
	}
	void reset(){
		fill(state.begin(),state.end(),false);
	}
private:
	mutex ms;
	vector<bool> state;
};
struct ConditionGeneral:public ReplyHandler::Condition{
	ConditionGeneral(const vector<int>& expect):
			expected(expect),state(expected){}
	ConditionGeneral(vector<int>&& expect):
			expected(move(expect)),state(expected){}
	bool update(const int source){
		lock_guard<mutex> lg(ms);
		--state[source];
		if(all_of(state.begin(), state.end(),[](const int v){return v<=0;})){
			return true;
		}
		return false;
	}
	void reset(){
		state=expected;
	}
private:
	mutex ms;
	vector<int> expected;
	vector<int> state;
};

ReplyHandler::Condition* ReplyHandler::condFactory(const ConditionType ct)
{
	if(ct==ANY_ONE){
		return new ConditionAny();
	}
	return new Condition();
}
ReplyHandler::Condition* ReplyHandler::condFactory(
		const ConditionType ct, const int numSource)
{
	if(ct==EACH_ONE){
		return new ConditionEachOne(numSource);
	}
	return new Condition();
}
ReplyHandler::Condition* ReplyHandler::condFactory(
		const ConditionType ct, const std::vector<int>& expected)
{
	if(ct==GENERAL){
		return new ConditionGeneral(expected);
	}
	return new Condition();
}
ReplyHandler::Condition* ReplyHandler::condFactory(
		const ConditionType ct, std::vector<int>&& expected)
{
	if(ct==GENERAL){
		return new ConditionGeneral(expected);
	}
	return new Condition();
}

/*
 * ReplyHandler::Item part:
 */

ReplyHandler::Item::Item():
		cond(nullptr),spwanThread(false),activated(true){}
ReplyHandler::Item::Item(const Item& i):fn(i.fn), cond(nullptr),
		spwanThread(i.spwanThread),activated(i.activated)
{
	std::swap(cond,const_cast<Item&>(i).cond);
}
ReplyHandler::Item::Item(Item&& i):fn(move(i.fn)),cond(nullptr),
		spwanThread(i.spwanThread),activated(i.activated)
{
	std::swap(cond,i.cond);
}
ReplyHandler::Item::Item(std::function<void()> f, Condition* c, const bool st):
	fn(f), cond(c), spwanThread(st), activated(true){}

ReplyHandler::Item::~Item(){
	delete cond;
}


/*
 * ReplyHandler part:
 */

bool ReplyHandler::input(const int type, const int source){
	auto it=cont.find(type);
	if(it==cont.end() || !it->second.activated )	return false;
	if(it->second.cond->update(source)){
		launch(it->second);
	}
	return true;
}

void ReplyHandler::addType(const int type, Condition* cond,
		std::function<void()> fn, const bool spwanThread)
{
//	cont[type]=Item(fn, cond, spwanThread);
	cont.insert(make_pair(type, move(Item(fn, cond, spwanThread))));
}
void ReplyHandler::removeType(const int type){
	auto it=cont.find(type);
	if(it != cont.end()) {
		delete it->second.cond;
		cont.erase(it);
	}
}

void ReplyHandler::clear(){
	for(auto& p : cont) {
		delete p.second.cond;
	}
	cont.clear();
}

void ReplyHandler::launch(Item& item){
	if(!item.spwanThread){
		item.fn();
	}else{
		thread t(item.fn);
		t.detach();
	}
}
