#include "stdafx.h"
#include "StrategyOFG.h"

using namespace std;


std::vector<Edge> StrategyOFG::initialCandidateEdges()
{
	vector<Edge> res;
	for(int s = 0; s < nNode; ++s) {
		for(int d = s + 1; d < nNode; ++d) {
			if(testEdgeXSub(s, d, *pgp, 0.001))
				res.emplace_back(s, d);
		}
	}
	return res;
}

int StrategyOFG::maintainDCESConnected(std::vector<Edge>& edges, std::vector<bool>& used)
{
	size_t f = 0;
	for(size_t i = 0; i < used.size(); ++i) {
		if(used[i] == true) {
			if(f == i)
				++f;
			else
				edges[f++] = move(edges[i]);
		}
	}
	int removed = used.size() - f;
	edges.erase(edges.begin() + f, edges.end());
	return removed;
}

int StrategyOFG::maintainDCESBound(std::vector<Edge>& edges,
	std::vector<double>& edgeFreq, const double worst)
{
	size_t f = 0;
	for(size_t i = 0; i < edges.size(); ++i) {
		if(edgeFreq[i] >= worst) {
			if(f == i)
				++f;
			else
				edges[f++] = move(edges[i]);
		}
	}
	int removed = edges.size() - f;
	edges.erase(edges.begin() + f, edges.end());
	return removed;
}

std::pair<std::vector<Edge>, std::vector<double>> StrategyOFG::getExistedEdges(
	const std::vector<std::vector<Graph>>& subs) const
{
	std::vector<Edge> edges;
	std::vector<double> freqs;
	double factor = 1.0 / subs.size();
	for(int i = 0; i < nNode; ++i) {
		for(int j = 0; j < nNode; ++j) {
			int t = countEdgeXSub(i, j, subs);
			if(t != 0) {
				auto f = t*factor;
				edges.emplace_back(i, j);
				freqs.push_back(f);
			}
		}
	}
	return make_pair(move(edges), move(freqs));
}


