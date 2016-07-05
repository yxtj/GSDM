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
	static const std::string optName;
	static const std::string usagePrefix;

	using createFun = std::function<CandidateMethod*()>;
	static std::map<std::string, createFun> contGen;
	static std::map<std::string, std::string> contUsage;
public:
	template <class T>
	static void registerClass(const std::string& name);
	static void registerUsage(const std::string& name, const std::string& usage);

	template <class T>
	static void registerClass();
	template <class T>
	static void registerUsage();

	static bool isValid(const std::string& name);
	static void init();
	static std::string getOptName();
	static std::string getUsage();

	static CandidateMethod* generate(const std::string& methodName);
	static CandidateMethod* generate(const CandidateMethodParam& methodParam);
};

template <class T>
void CandidateMethodFactory::registerClass() {
	contGen[T::name] = []() {
		return new T();
	};
}

template <class T>
void CandidateMethodFactory::registerClass(const std::string& name) {
	contGen[name] = []() {
		return new T();
	};
}

template <class T>
void CandidateMethodFactory::registerUsage() {
	registerUsage(T::name, T::usage);
}
