#include "stdafx.h"
#include "CandidateMthdFreq.h"

#include <iostream>

using namespace std;

const std::string CandidateMthdFreq::name("Freq");

CandidateMthdFreq::CandidateMthdFreq()
{
}

void CandidateMthdFreq::setMotifSize(const int smin, const int smax)
{
	this->smin = smin;
	this->smax = smax;
}

void CandidateMthdFreq::setParam(const CandidateMethodParm & par)
{
	this->par = &static_cast<const CandidateMthdFreqParm&>(par);
}

void CandidateMthdFreq::setGraphSet(const std::vector<Graph>& gs)
{
	this->gs = &gs;
	gp.init(gs);
	this->nNode = gs.front().nNode;
}

std::vector<std::pair<Motif, double>> CandidateMthdFreq::getCandidantMotifs(const std::vector<Graph>& gs,
	const int smin, const int smax, const CandidateMethodParm& param)
{
	setMotifSize(smin, smax);
	setParam(param);
	setGraphSet(gs);

//	cout << "test in StrategyFreq::motifOnIndTopK" << endl;
//	cout << par->pMin << endl;

	
	vector<pair<Motif, double>> mps = method_edge2_dp();

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


std::vector<Edge> CandidateMthdFreq::getEdges(const GraphProb & gp)
{
	vector<Edge> edges;
	for(int i = 0; i < nNode; ++i) {
		for(int j = i + 1; j < nNode; ++j)
			if(gp.matrix[i][j] >= par->pMin)
				edges.emplace_back(i, j);
	}
	return edges;
}



// ---------------------------- Parameter Class: ----------------------------

void CandidateMthdFreqParm::construct(
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
