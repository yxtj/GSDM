#include "stdafx.h"
#include "StrategyFactory.h"
#include "Option.h"

#include "StrategyCandidate.h"

using namespace std;

const std::string StrategyFactory::optName("strategy");
const std::string StrategyFactory::usagePrefix(
	"Use the following parameters to select ONE learning strategy\n");

std::map<std::string, StrategyFactory::createFun> StrategyFactory::contGen{};
std::map<std::string, std::string> StrategyFactory::contUsage{};

bool StrategyFactory::isValid(const std::string & name)
{
	return contGen.find(name) != contGen.end();
}

void StrategyFactory::init()
{
	registerClass<StrategyCandidate>();
	registerUsage<StrategyCandidate>();
	// TODO: add new strategy here
	//	registerClass<CandidateMthdInfreq>();
}

void StrategyFactory::registerUsage(const std::string& name, const std::string& usage)
{
	contUsage[name] = usage;
}

std::string StrategyFactory::getOptName()
{
	return optName;
}

std::string StrategyFactory::getUsage()
{
	string res = usagePrefix;
	int cnt = 0;
	for(const auto& usg : contUsage) {
		res += "Strategy " + to_string(++cnt) + ": " + usg.first
			+ "\n" + usg.second + "\n";
	}
	return res;
}

StrategyBase * StrategyFactory::generate(const std::string & name)
{
	StrategyBase* res = nullptr;
	if(isValid(name)) {
		res = contGen.at(name)();
	}
	return res;
}
