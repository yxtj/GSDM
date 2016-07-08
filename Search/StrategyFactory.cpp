#include "stdafx.h"
#include "StrategyFactory.h"
#include "Option.h"

// TODO: add new strategy headers
#include "StrategyCandidate.h"
#include "StrategyCandidatePN.h"
#include "StrategySample.h"
#include "StrategyXorFreq.h"
#include "StrategyParaFreqPmN.h"

using namespace std;

using parent_t = StrategyFactory::parent_t;

template <>
const std::string parent_t::optName("strategy");
template <>
const std::string parent_t::usagePrefix(
	"Use the following parameters to select ONE learning strategy\n");

template <>
std::map<std::string, parent_t::createFun> parent_t::contGen{};
template <>
std::map<std::string, std::string> parent_t::contUsage{};


void StrategyFactory::init()
{
	// TODO: add new strategy here
	registerInOne<StrategyCandidate>();
	registerInOne<StrategyCandidatePN>();
	registerInOne<StrategySample>();
	registerInOne<StrategyXorFreq>();
	registerInOne<StrategyParaFreqPmN>();
}

StrategyBase * StrategyFactory::generate(const std::string & name)
{
	FactoryProductTemplate* p = parent_t::generate(name);
	StrategyBase* res = dynamic_cast<StrategyBase*>(p);
	return res;
}
