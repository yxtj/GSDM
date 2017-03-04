#include "stdafx.h"
#include "StrategyBase.h"

using namespace std;

bool StrategyBase::checkInput(const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg)
{
	return !gPos.empty() && !gPos.front().empty()
		&& !gNeg.empty() && !gNeg.front().empty();
}

bool StrategyBase::checkInput(const DataHolder & dPos, const DataHolder & dNeg)
{
	return !dPos.empty() && !dNeg.empty();
}

