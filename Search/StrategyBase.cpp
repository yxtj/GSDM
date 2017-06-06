#include "stdafx.h"
#include "StrategyBase.h"

using namespace std;

bool StrategyBase::checkInput(const DataHolder & dPos, const DataHolder & dNeg)
{
	return !dPos.empty() || !dNeg.empty();
}

