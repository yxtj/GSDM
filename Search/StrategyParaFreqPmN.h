#pragma once
#include "CandidateMethod.h"
#include "StrategyBase.h"
#include <vector>

class Option;

class StrategyParaFreqPmN
	: public StrategyBase
{
	double pRefine;
	double pPickNeg;
	double pRefineNeg;

public:
	static const std::string name;
	static const std::string usage;

	StrategyParaFreqPmN() = default;

	virtual bool parse(const std::vector<std::string>& param);

	virtual std::vector<Motif> search(const Option& opt,
		const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg);

private:

	std::unordered_map<Motif, std::pair<int, double>> freqOnSet(
		CandidateMethod* method, const std::vector<std::vector<Graph>>& gPos, const std::vector<int>& blacklist);

	void countMotif(std::unordered_map<Motif, std::pair<int, double>>& res, std::vector<std::pair<Motif, double>> &vec);

	std::vector<Motif> refineByPositive(std::unordered_map<Motif, std::pair<int, double>>& data,
		const size_t gsize, const int k = std::numeric_limits<int>::max());

	std::vector<Motif> filterByNegative(std::vector<Motif>& motifs,
		const std::vector<std::vector<Graph>>& gNeg);
};
