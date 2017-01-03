#include "stdafx.h"
#include "StrategyOFG.h"

using namespace std;


void StrategyOFG::setDCESmaintainOrder(bool inorder)
{
	if(inorder == true) {
		maintainDCESConnected = &StrategyOFG::maintainDCESConnected_inorder;
		maintainDCESBound = &StrategyOFG::maintainDCESBound_inorder;
	}else{
		maintainDCESConnected = &StrategyOFG::maintainDCESConnected_unorder;
		maintainDCESBound = &StrategyOFG::maintainDCESBound_unorder;
	}
}

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

std::vector<std::pair<Edge, double>> StrategyOFG::getExistedEdges(
	const std::vector<std::vector<Graph>>& subs) const
{
	std::vector<std::pair<Edge, double>> res;
	double factor = 1.0 / subs.size();
	for(int i = 0; i < nNode; ++i) {
		for(int j = 0; j < nNode; ++j) {
			int t = countEdgeXSub(i, j, subs);
			if(t != 0) {
				auto f = t*factor;
				res.emplace_back(Edge(i, j), f);
			}
		}
	}
	return res;
}

int StrategyOFG::maintainDCESConnected_inorder(std::vector<std::pair<Edge, double>>& edges, std::vector<bool>& used)
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

int StrategyOFG::maintainDCESConnected_unorder(std::vector<std::pair<Edge, double>>& edges, std::vector<bool>& used)
{
	size_t f = 0, l = edges.size();
	while(l>0 && !used[l - 1])
		--l;
	while(f < l) {
		if(used[f]) {
			++f;
		} else {
			edges[f++] = edges[--l];
			while(l>f && !used[l - 1])
				--l;
		}
	}
	int removed = edges.size() - l;
	edges.erase(edges.begin() + l, edges.end());
	return removed;
}

int StrategyOFG::maintainDCESBound_inorder(std::vector<std::pair<Edge, double>>& edges, const double worst)
{
	auto it = remove_if(edges.begin(), edges.end(), [=](pair<Edge, double>& p) {
		return p.second < worst;
	});
	int removed = edges.end() - it;
	edges.erase(it, edges.end());
	return removed;
}

int StrategyOFG::maintainDCESBound_unorder(std::vector<std::pair<Edge, double>>& edges, const double worst)
{
	size_t f = 0, l = edges.size();
	while(l>0 && edges[l-1].second<worst)
		--l;
	while(f < l) {
		if(edges[l-1].second>=worst) {
			++f;
		} else {
			edges[f++] = edges[--l];
			while(l>f && edges[l - 1].second<worst)
				--l;
		}
	}
	int removed = edges.size() - l;
	edges.erase(edges.begin() + l, edges.end());
	return removed;
}



