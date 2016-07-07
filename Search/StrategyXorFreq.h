#pragma once
#include "Graph.h"
#include "Motif.h"
#include "CandidateMethod.h"
#include "StrategyBase.h"
#include <vector>


class StrategyXorFreq
	: public StrategyBase
{
	double pRefine;
public:
	static const std::string name;
	static const std::string usage;

	StrategyXorFreq() = default;

	virtual bool parse(const std::vector<std::string>& param);

	virtual std::vector<Motif> search(const Option& opt,
		const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg);

private:
	std::unordered_map<Motif, std::pair<int, double>> freqOnSet(CandidateMethod* method, const std::vector<std::vector<Graph>>& gPos);

	void countMotif(std::unordered_map<Motif, std::pair<int, double>>& res, std::vector<std::pair<Motif, double>> &vec);

	std::vector<Motif> pickTopK(std::unordered_map<Motif, std::pair<int, double>>& data, const size_t gsize);

	std::vector<Motif> symmetricDifference(std::vector<Motif>& a, std::vector<Motif>& b);

	// pick the top k motifs from given candidate set, where each one with a minimum probability
	// return <motif, prob. showing up over individuals, mean prob. of happening of single individual>
	std::vector<std::tuple<Motif, double, double>> refineByAll(
		const std::vector<std::vector<std::pair<Motif, double>>>& motifs);
};
