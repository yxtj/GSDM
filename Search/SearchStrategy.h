#pragma once
#include "Motif.h"
#include "Graph.h"

struct SearchStrategyPara;

class SearchStrategy
{
public:
	SearchStrategy();
	~SearchStrategy();

	virtual std::vector<std::pair<Motif, double>> getCandidantMotifs(const std::vector<Graph> & gs,
		const int smin, const int smax, const SearchStrategyPara& par) = 0;

	static double probOfMotif(const Motif& m, const std::vector<Graph>& gs);
	static double probOfMotif(const Motif& m, const int newS, const int newD, const std::vector<Graph>& gs);
};

struct SearchStrategyPara 
{
	SearchStrategyPara() = default;
	SearchStrategyPara(const SearchStrategyPara&) = default;
	SearchStrategyPara(SearchStrategyPara&&) = default;

	virtual void construct(const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg) = 0;
};
