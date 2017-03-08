#pragma once
#include "StrategyOFGPara.h"
class StrategyPeriodic :
	public StrategyOFGPara
{
public:

public:
	static const std::string name;
	static const std::string usage;

	virtual bool parse(const std::vector<std::string>& param);

	//virtual bool explore(const Motif& m);
	virtual std::pair<double, double> scoring(const MotifBuilder& mb, const double lowerBound);
protected:
};

