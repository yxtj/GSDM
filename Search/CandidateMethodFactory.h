#pragma once
#include "CandidateMethod.h"
#include <string>
#include <map>
#include <functional>

/************************************************************************/
/* extension:
	step 1, add a name for the new strategy in its own definition
	step 2, register the new strategy into this factory at init()
*/
/************************************************************************/ 

class CandidateMethodFactory
{
	using createFun = std::function<CandidateMethod*()>;
	static std::map<std::string, createFun> cont;
public:
	static bool isValid(const std::string& name);
	template <class T>
	static void registerStrategy();
	static void init();

	static CandidateMethod* generate(const std::string& strategyName);
};

template <class T>
void CandidateMethodFactory::registerStrategy() {
	cont[T::name] = []() {
		return new T();
	};
}

