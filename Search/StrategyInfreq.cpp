#include "stdafx.h"
#include "StrategyInfreq.h"

#include <iostream>

using namespace std;

const std::string StrategyInfreq::name("Infreq");

StrategyInfreq::StrategyInfreq()
{
}

std::vector<std::pair<Motif, double>> StrategyInfreq::getCandidantMotifs(const std::vector<Graph>& gs,
	const int smin, const int smax, const SearchStrategyPara& par)
{
	this->smin = smin;
	this->smax = smax;
	const StrategyInfreqPara& param = static_cast<const StrategyInfreqPara&>(par);
	this->par = &param;
	GraphProb gp(gs);
	this->nNode = gp.nNode;
	//	cout << "test in StrategyInfreq::motifOnIndTopK" << endl;
	//	cout << param.pMin << endl;


	vector<pair<Motif, double>> mps;
	pair<Motif, double> dummy;
	dummy.second = 1.0;
	for(int s = 0; s < nNode; ++s) {
		auto t = dfsMotif4(dummy, s, gs, gp);
		for(auto it = t.begin(); it != t.end(); ++it)
			if(it->first.getnEdge() >= smin)
				mps.push_back(move(*it));
	}

	sort(mps.begin(), mps.end());
	auto itend = unique(mps.begin(), mps.end());
	cout << mps.end() - itend << " / " << mps.size() << " redundant motifs " << endl;
	mps.erase(itend, mps.end());

	this->par = nullptr;
/*	// Pick the top K result:
	if(mps.size() > k) {
		partial_sort(mps.begin(), mps.begin()+k, mps.end(),
		[](const pair<Motif, double>& a, const pair<Motif, double>& b) {
			return a.second < b.second;
		});
		mps.erase(mps.begin() + k, mps.end());
	}
*/
	return mps;
}

// node expansion
/*
Return: the valid motifs generated from curr by expanding at expNode (exclude curr)
Precondition:
curr.first is a valid motif (prob.>=par->pMin && size<=smax)
expNode is contained by curr.first
Postcondition:
all return motifs contains curr.first and
*/
std::vector<std::pair<Motif, double>> StrategyInfreq::dfsMotif4(
	const std::pair<Motif, double>& curr, const int expNode,
	const std::vector<Graph>& gs, const GraphProb & gp)
{
	std::vector<std::pair<Motif, double>> res;
	res.push_back(curr);
	if(curr.first.getnEdge() >= smax) {
		return res;
	}
	// by now curr.first.getnEdge() <= smax-1 (expandable)
	// expand to each valid neighbor
	for(int i = 0; i < nNode; ++i) {
		// pre-prune: only expand to infrequent edges
		if(gp.matrix[expNode][i] > par->pMin && !curr.first.containEdge(expNode, i))
			continue;
		size_t currResSize = res.size();
		for(size_t j = 0; j < currResSize; ++j) {
			auto& mp = res[j];
			if(mp.first.getnEdge() < smax
				&& (mp.first.getnEdge() == 0 || mp.first.lastEdge() < Edge{ expNode, i })) {
				Motif m(mp.first);
				m.addEdge(expNode, i);
				double p = probOfMotif(m, gs);
				if(p <= par->pMin) {
					auto next = make_pair(move(m), p);
					auto t = dfsMotif4(next, i, gs, gp);
//					res.reserve(res.size() + t.size() + 1);
//					res.push_back(move(next));
					res.reserve(res.size() + t.size());
					res.insert(res.end(), make_move_iterator(t.begin()), make_move_iterator(t.end()));
				}
			}
		}
	}
	return res;
}

// layer expansion
std::vector<std::pair<Motif, double>> StrategyInfreq::dfsMotif5(const std::pair<Motif, double>& curr, const int expNode, const std::vector<Graph>& gs, const GraphProb & gp)
{
	return std::vector<std::pair<Motif, double>>();
}



// ---------------------------- Parameter Class: ----------------------------

void StrategyInfreqPara::construct(
	const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg)
{
	//gp.startAccum(gPos.front().size());
	//for(const auto& line : gPos) {
	//	for(const Graph& g : line) {
	//		gp.iterAccum(g);
	//	}
	//}
	//gp.finishAccum();
}
