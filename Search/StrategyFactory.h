#pragma once
#include "StrategyBase.h"
#include "FactoryTemplate.h"
#include <string>

class StrategyFactory
	: public FactoryTemplate<StrategyBase>
{
	static void registerDesc(const std::string& name, const std::string& desc);
public:
	using parent_t = FactoryTemplate<StrategyBase>;

	static std::map<std::string, std::string> contDesc;

	static void init();

	template <class T>
	static void reg();

	static StrategyBase* generate(const std::string& name);
};

template<class T>
inline void StrategyFactory::reg()
{
	registerInOne<T>();
	//registerDesc(T::name, T::desc);
}
