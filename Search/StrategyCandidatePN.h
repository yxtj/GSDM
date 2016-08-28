#pragma once
#include "CandidateMethod.h"
#include "StrategyBase.h"
#include "../common/GraphProb.h"
#include <vector>

class Option;

class StrategyCandidatePN
	: public StrategyBase
{
	int k;
	double pRefine;
	double pPickNeg;
	double pRefineNeg;

public:
	static const std::string name;
	static const std::string usage;

	StrategyCandidatePN() = default;

	virtual bool parse(const std::vector<std::string>& param);

	virtual std::vector<Motif> search(const Option& opt,
		const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg);

private:
	// learn candidate motifs from given raw dataset
	std::vector<std::pair<Motif, double>> candidateFromOne(
		CandidateMethod* method, const std::vector<Graph> & gs);

	std::unordered_map<Motif, std::pair<int, double>> freqOnSet(
		CandidateMethod* method, const std::vector<std::vector<Graph>>& gPos);

	void countMotif(std::unordered_map<Motif, std::pair<int, double>>& res, std::vector<std::pair<Motif, double>> &vec);

	std::vector<Motif> pickTopK(std::unordered_map<Motif, std::pair<int, double>>& data,
		const size_t gsize, const int k = std::numeric_limits<int>::max());

	// pick the top k motifs from given candidate set, where each one with a minimum probability
	// return <motif, prob. showing up over individuals, mean prob. of happening of single individual>
	std::vector<std::tuple<Motif, double, double>> refineByAll(
		std::vector<std::vector<std::pair<Motif, double>>>& motifs);

	std::vector<Motif> filterByNegative(std::vector<Motif>& motifs,
		const std::vector<std::vector<Graph>>& gNeg);
};

