#pragma once
#include "SearchStrategy.h"
#include "Motif.h"
#include "Graph.h"
#include "GraphProb.h"
#include <vector>
#include <utility>
#include <functional>

struct StrategyInfreqPara;

class StrategyInfreq :
	public SearchStrategy
{
	int nNode; // size of original graph
	int smin, smax;// size of motif [smin, smax]

	const StrategyInfreqPara* par;
public:
	static const std::string name;
	StrategyInfreq();

	virtual std::vector<std::pair<Motif, double>> getCandidantMotifs(const std::vector<Graph> & gs,
		const int smin, const int smax, const SearchStrategyPara& par);
private:
	std::vector<std::pair<Motif, double>> dfsMotif4(
		const std::pair<Motif, double>& curr, const int expNode,
		const std::vector<Graph>& gs, const GraphProb& gp);
	std::vector<std::pair<Motif, double>> dfsMotif5(
		const std::pair<Motif, double>& curr, const int expNode,
		const std::vector<Graph>& gs, const GraphProb& gp);
};

struct StrategyInfreqPara :
	public SearchStrategyPara
{
	double pMin;
	std::function<bool(double, double)> op_freq = std::less<double>();
	virtual void construct(const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg);
};
