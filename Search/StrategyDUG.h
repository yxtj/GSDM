#pragma once
#include "StrategyBase.h"

// Strategy for Discriminative Uncertain Graph
class StrategyDUG
	: public StrategyBase
{
	int k;
public:
	static const std::string name;
	static const std::string usage;

	virtual bool parse(const std::vector<std::string>& param);

	virtual std::vector<Motif> search(const Option& opt,
		const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg);

};

