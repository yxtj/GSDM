#pragma once
#include "Graph.h"
#include "Motif.h"
#include "ComplexParamBase.h"
#include <vector>
#include <string>

struct StrategyBaseParam
	: public ComplexParamBase
{};

class StrategyBase
{
public:
	const std::string name;
public:
	StrategyBase();
	
	virtual std::vector<std::tuple<Motif, double, double>> search(
		const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg) = 0;

	virtual ~StrategyBase();
};

