#pragma once
#include "Graph.h"
#include "Motif.h"
#include "FactoryProductTemplate.h"
#include <vector>
#include <string>

class Option;

class StrategyBase
	: public FactoryProductTemplate
{
public:
	bool checkInput(const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg);

	virtual bool parse(const std::vector<std::string>& param) = 0;
	
	virtual std::vector<Motif> search(const Option& opt,
		const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg) = 0;

};

