#pragma once
#include "FactoryProductTemplate.h"
#include <string>
#include <vector>
#include <map>
#include <functional>

class FactoryTemplate
{
	static std::string optName;
	static std::string usagePrefix;

	using createFun = std::function<FactoryProductTemplate*()>;
	static std::map<std::string, createFun> contGen;
	static std::map<std::string, std::string> contUsage;
public:
	FactoryTemplate() = delete;

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
	static void setOptName(const std::string& name);
	static std::string getUsage();
	static void setUsage(const std::string& usage);

	static FactoryProductTemplate* generate(const std::string& name);
};

template <class T>
void FactoryTemplate::registerClass(const std::string& name) {
	contGen[name] = []() {
		return new T();
	};
}

template <class T>
void FactoryTemplate::registerClass() {
	registerClass<T>(T::name);
	//contGen[T::name] = []() {
	//	return new T();
	//};
}

template <class T>
void FactoryTemplate::registerUsage() {
	registerUsage(T::name, T::usage);
}
