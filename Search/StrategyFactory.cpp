#include "stdafx.h"
#include "StrategyFactory.h"
#include "Option.h"

#include "StrategyCandidate.h"

using namespace std;

void StrategyFactory::init()
{
	setOptName("strategy");
	setUsage("Use the following parameters to select ONE learning strategy\n");
	// TODO: add new strategy here
	registerClass<StrategyCandidate>();
	registerUsage<StrategyCandidate>();
}

StrategyBase * StrategyFactory::generate(const std::string & name)
{
	FactoryProductTemplate* p = FactoryTemplate::generate(name);
	StrategyBase* res = dynamic_cast<StrategyBase*>(p);
	return res;
}
