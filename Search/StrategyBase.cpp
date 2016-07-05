#include "stdafx.h"
#include "StrategyBase.h"

using namespace std;

void StrategyBase::checkNumber(int required, size_t given, const std::string& name)
{
	if(given != required + 1) {
		throw invalid_argument("Wrong number of sub-parameters for strategy: "
			+ name + ". " + to_string(required + 1) + " required, but "
			+ to_string(given) + " is given.");
	}
}

void StrategyBase::checkName(const std::vector<string>& param, const std::string & name) noexcept(false)
{
	if(param[0] != name) {
		throw invalid_argument("The first strategy parameter (" + param.front()
			+ ") do not match current strategy name (" + name + ")");
	}
}


void StrategyBase::checkParam(const std::vector<string>& param, int reqired, const std::string & name) noexcept(false)
{
	checkNumber(reqired, param.size(), name);
	checkName(param, name);
}

bool StrategyBase::checkInput(const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg)
{
	return !gPos.empty() && !gPos.front().empty()
		&& !gNeg.empty() && !gNeg.front().empty();
}

StrategyBase::~StrategyBase()
{
}
