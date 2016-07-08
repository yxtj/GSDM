#pragma once
//#include "FactoryProductTemplate.h"
#include <string>
#include <vector>
#include <map>
#include <functional>

/*
Template class of simple factory which works with the Option.
PRDCT should be a derived class of FactoryProductTemplate
*/
template <class PRDCT>
class FactoryTemplate
{
protected:
	//using createFun = std::function<FactoryProductTemplate*()>;
	using createFun = std::function<PRDCT*()>;
	static std::map<std::string, createFun> contGen;
	static std::map<std::string, std::string> contUsage;
public:
	// used in setting program option
	// specialization class should give defination of these two values
	static const std::string optName;
	static const std::string usagePrefix;

	FactoryTemplate() = delete;

	template <class T>
	static void registerClass(const std::string& name);
	template <class T>
	static void registerClass();

	static void registerUsage(const std::string& name, const std::string& usage);
	template <class T>
	static void registerUsage();

	template <class T>
	static void registerInOne();


	static std::string getUsage();

	static bool isValid(const std::string& name);
	// specialization class should give implement following function
	//static void init();

	static FactoryProductTemplate* generate(const std::string& name);
};


template <class PRDCT>
template <class T>
inline void FactoryTemplate<PRDCT>::registerClass(const std::string& name) {
	contGen[name] = []() {
		return new T();
	};
}

template <class PRDCT>
template <class T>
inline void FactoryTemplate<PRDCT>::registerClass() {
	registerClass<T>(T::name);
}

template<class PRDCT>
inline void FactoryTemplate<PRDCT>::registerUsage(const std::string & name, const std::string & usage) {
	contUsage[name] = usage;
}

template <class PRDCT>
template <class T>
inline void FactoryTemplate<PRDCT>::registerUsage() {
	registerUsage(T::name, T::usage);
}

template<class PRDCT>
template<class T>
inline void FactoryTemplate<PRDCT>::registerInOne()
{
	registerClass<T>();
	registerUsage<T>();
}

template<class PRDCT>
std::string FactoryTemplate<PRDCT>::getUsage()
{
	std::string res = usagePrefix;
	int cnt = 0;
	for(const auto& usg : contUsage) {
		res += "Option " + std::to_string(++cnt) + ": " + usg.first
			+ "\n" + usg.second + "\n";
	}
	return res;
}

template<class PRDCT>
inline bool FactoryTemplate<PRDCT>::isValid(const std::string & name) {
	return contGen.find(name) != contGen.end();
}

template<class PRDCT>
inline FactoryProductTemplate * FactoryTemplate<PRDCT>::generate(const std::string & name)
{
	FactoryProductTemplate* res = nullptr;
	if(isValid(name)) {
		res = contGen.at(name)();
	}
	return res;
}
