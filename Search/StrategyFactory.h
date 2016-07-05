#pragma once
#include "StrategyBase.h"
#include <string>
#include <map>
#include <functional>

class Option;

class StrategyFactory
{
	static const std::string optName;
	static const std::string usagePrefix;

	using createFun = std::function<StrategyBase*()>;
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

	static StrategyBase* generate(const std::string& name);
};

template <class T>
void StrategyFactory::registerClass(const std::string& name) {
	contGen[name] = []() {
		return new T();
	};
}

template <class T>
void StrategyFactory::registerClass() {
	registerClass<T>(T::name);
	//contGen[T::name] = []() {
	//	return new T();
	//};
}

template <class T>
void StrategyFactory::registerUsage() {
	registerUsage(T::name, T::usage);
}
