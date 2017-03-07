#pragma once
#include "../common/Graph.h"
#include "../common/Motif.h"
#include "DataHolder.h"
#include "FactoryProductTemplate.h"
#include <vector>
#include <string>

class Option;

class StrategyBase
	: public FactoryProductTemplate
{
public:
	bool checkInput(const DataHolder& dPos, const DataHolder& dNeg);

	virtual bool parse(const std::vector<std::string>& param) = 0;
	
	virtual std::vector<Motif> search(const Option& opt,
		DataHolder& dPos, DataHolder& dNeg) = 0;

};

