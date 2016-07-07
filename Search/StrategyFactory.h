#pragma once
#include "StrategyBase.h"
#include "FactoryTemplate.h"
#include <string>

class StrategyFactory
	: public FactoryTemplate<StrategyBase>
{
public:
	using parent_t = FactoryTemplate<StrategyBase>;

	static void init();

	static StrategyBase* generate(const std::string& name);
};
