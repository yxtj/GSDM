#pragma once
#include "Graph.h"
#include "Motif.h"
#include "CandidateMethod.h"
#include "StrategyBase.h"
#include <vector>

class StrategyFuncFreq
	: public StrategyBase
{
	// input options:
	int k; // number of result
	double alpha; // penalty for negative frequency
	double minSup; // minium show up probability among postive subjects
//	int smin, smax; // minimum/maximum motif size

// local parameters shared by internal functions (valid during searching is called)
	int nNode;
	int nMinSup;
	std::vector<double> topKScores;
	const std::vector<std::vector<Graph>>* pgp, *pgn;
public:
	static const std::string name;
	static const std::string usage;

	StrategyFuncFreq() = default;

	virtual bool parse(const std::vector<std::string>& param);

	virtual std::vector<Motif> search(const Option& opt,
		const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg);

	double objectFunction(const double freqPos, const double freqNeg);
private:
	std::vector<Motif> method_edge2_dp();
	std::vector<MotifBuilder> _edge2_dp(
		const std::vector<MotifBuilder>& last, const Edge& e);
};

