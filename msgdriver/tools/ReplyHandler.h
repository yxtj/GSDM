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
 * This is a helper class for handling reply message. It invokes registered
 * handler when a certain condition satifies.
 * The "Reply" means all the input information is just the source and type.
 * One reply is an input as (source, type) pair.
 * It can be used with out class MsgDriver or class Dipatcher.
 * Usage:
 *  1, register types, their handlers and invoke conditions using addType()
 *  2, input the type and source of with input()
 *  3, when a condition fulfills, its hanlder is called automatically
 *  4, reset the condition with resetTypeCondition(), if you want to use it 
 *     after being called once.
 * Condition for trigger a handler.
 *  1, used to determine when a handler should be called.
 *  2, three types of pre-defined Conditions in condFactory()
 *  2.1, ANY_ONE: triggers after a reply from any source.
 * 		Constructed with NO parameter.
 *  2.2, EACH_ONE: triggers after each source have replied AT LEAST once.
 *  	Constructed with parameter $numSource$. (Source index starts from 0)
 *  2.3, GERNERAL: triggers after each source have replied AT LEAST
 *  	expected times.
 *  	Constructed with parameter $expected$. (Source index starts from 0)
 *  3, RESET the condition if you want to use it again after triggered once.
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

	enum ConditionType{
		ANY_ONE, EACH_ONE, GENERAL
	};

	// create an ANY_ONE condition: one reply from any source
	static Condition* condFactory(const ConditionType ct);
	// create an EACH_ONE condition: at least a reply for each source from 0 to num-1
	static Condition* condFactory(
			const ConditionType ct, const int numSource);
	// create a GENERAL condition: given specific number of replies for each source
	static Condition* condFactory(
			const ConditionType ct, const std::vector<int>& expected);
	// create a GENERAL condition: given specific number of replies for each source
	static Condition* condFactory(
			const ConditionType ct, std::vector<int>&& expected);

	// return whether this input is handled by this calling
	bool input(const int type, const int source);

	// register a type, handler and condition, and activate it
	void addType(const int type, Condition* cond,
			std::function<void()> fn, const bool spwanThread=false);
	void removeType(const int type);

	void activateType(const int type){
		cont.at(type).activated=true;
	}
	void deactivateType(const int type){
		cont.at(type).activated=false;
	}
	void resetTypeCondition(const int type){
		cont.at(type).cond->reset();
	}

	void clear();
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
