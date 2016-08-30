#pragma once
/*
 * ReplyHandler.h
 *
 *  Created on: Dec 20, 2015
 *      Author: tzhou
 */

#ifndef DRIVER_TOOLS_REPLYHANDLER_H_
#define DRIVER_TOOLS_REPLYHANDLER_H_

#include <unordered_map>
#include <functional>
#include <vector>

/*
 * This is a helper class for handling reply message. It can be used with out
 * class MsgDriver or class Dipatcher.
 * Usage:
 *  1, register types of reply message you want to handle using addType()
 *  2, active that handler with activateType()
 *  3, input the type and source of a reply message with input()
 * Condition:
 *  1, used to check when the handle should be handled
 *  2, three types of pre-defined Conditions in condFactory()
 *  2.1, ANY_ONE: trigger handler after reply from any source.
 * 		constructed NO parameter.
 *  2.2, EACH_ONE: trigger handler after each source have replied AT LEAST once.
 *  	constructed with parameter numSource.
 *  2.3, GERNERAL: trigger handler after each source have replied AT LEAST
 *  	expected times.
 *  	constructed with parameter expected.
 */
class ReplyHandler{
public:
	struct Condition{
		//return whether condition is satisfied after receiving source.
		//should be able to reset itself after last satisfaction
		virtual bool update(const int source){return false;}
		virtual void reset(){}
		virtual ~Condition(){}
	};
	//return whether this input is handled by this calling
	bool input(const int type, const int source);

	enum ConditionType{
		ANY_ONE, EACH_ONE, GENERAL
	};

	static Condition* condFactory(const ConditionType ct);
	static Condition* condFactory(
			const ConditionType ct, const int numSource);
	static Condition* condFactory(
			const ConditionType ct, const std::vector<int>& expected);
	static Condition* condFactory(
			const ConditionType ct, std::vector<int>&& expected);

	void addType(const int type, Condition* cond,
			std::function<void()> fn, const bool spwanThread=false);
	void removeType(const int type);
	void activateType(const int type){
		cont.at(type).activated=true;
	}
	void pauseType(const int type){
		cont.at(type).activated=false;
	}
	void resetType(const int type){
		cont.at(type).cond->reset();
	}

	void clear(){ cont.clear(); }
	size_t size() const{ return cont.size(); }

private:
	struct Item{
		std::function<void()> fn;
		Condition* cond;	//its ownership is unique (only one object can hold it)
		bool spwanThread;
		bool activated;
		Item();
		Item(const Item& i);
		Item(Item&& i);
		Item(std::function<void()> f, Condition* c, const bool st);
		~Item();
	};
	std::unordered_map<int, Item> cont;

	static void launch(Item& item);
};

#endif /* DRIVER_TOOLS_REPLYHANDLER_H_ */
