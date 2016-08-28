#pragma once
#include "CandidateMethod.h"
#include "StrategyBase.h"
#include <vector>


class StrategyXor
	: public StrategyBase
{
	double pRefine;
public:
	static const std::string name;
	static const std::string usage;

	StrategyXor() = default;

	virtual bool parse(const std::vector<std::string>& param);

	virtual std::vector<Motif> search(const Option& opt,
		const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg);

private:
	std::unordered_map<Motif, std::pair<int, double>> freqOnSet(CandidateMethod* method, const std::vector<std::vector<Graph>>& gPos);

	void countMotif(std::unordered_map<Motif, std::pair<int, double>>& res, std::vector<std::pair<Motif, double>> &vec);

	std::vector<Motif> refineByPostive(std::unordered_map<Motif, std::pair<int, double>>& data,
		const size_t gsize, const size_t topk=std::numeric_limits<size_t>::max());

	std::vector<Motif> shuttfle(std::vector<Motif>& motifs);

	std::vector<Motif> symmetricDifference(std::vector<Motif>& a, std::vector<Motif>& b);

};
