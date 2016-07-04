#pragma once
#include "Graph.h"
#include "Motif.h"
#include "ComplexParamBase.h"
#include <vector>
#include <string>

class Option;

class StrategyBase
{
public:
	void checkParam(const std::vector<std::string>& param, int reqired, const std::string& name)
		noexcept(false); // throw exception if fails
	bool checkInput(const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg);

	virtual bool parse(const std::vector<std::string>& param) = 0;
	
	virtual std::vector<Motif> search(const Option& opt,
		const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg) = 0;

	virtual ~StrategyBase();

protected:
	void checkNumber(int required, size_t given, const std::string& name) noexcept(false); // throw exception if fails
	void checkName(const std::vector<std::string>& param, const std::string& name) noexcept(false); // throw exception if fails

};

