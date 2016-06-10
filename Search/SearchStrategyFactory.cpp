#include "stdafx.h"
#include "SearchStrategyFactory.h"
// include conclude strategies:
#include "StrategyFreq.h"
#include "StrategyInfreq.h"

using namespace std;

std::map<std::string, SearchStrategyFactory::createFun> SearchStrategyFactory::cont{};

bool SearchStrategyFactory::isValid(const std::string & name)
{
	return cont.find(name) != cont.end();
}

void SearchStrategyFactory::init()
{
	registerStrategy<StrategyFreq>();
	// TODO: add new strategy here
	registerStrategy<StrategyInfreq>();
}

SearchStrategy * SearchStrategyFactory::generate(const std::string & strategyName)
{
	SearchStrategy* res = nullptr;
	if(isValid(strategyName)) {
		res = cont.at(strategyName)();
	}
	return res;
}
