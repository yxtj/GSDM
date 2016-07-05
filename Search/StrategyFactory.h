#pragma once
#include "StrategyBase.h"
#include "FactoryTemplate.h"
#include <string>

class StrategyFactory
	: public FactoryTemplate
{
public:
	static void init();

	static StrategyBase* generate(const std::string& name);
};
