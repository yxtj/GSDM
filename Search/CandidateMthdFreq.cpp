#include "stdafx.h"
#include "CandidateMthdFreq.h"

#include <iostream>

using namespace std;

const std::string CandidateMthdFreq::name("freq");
const std::string CandidateMthdFreq::usage(
	"Find the frequent motifs with given size.\n"
	"  " + CandidateMthdFreq::name + " <min size> <max size> <min prob.>\n"
	"<min prob.>: the frequency threshold for accepted motifs");

CandidateMthdFreq::CandidateMthdFreq()
{
}

bool CandidateMthdFreq::parse(const std::vector<std::string>& param)
{
	try {
		checkParam(param, 3, name);
		smin = stoi(param[1]);
		smax = stoi(param[2]);
		pMin = stod(param[3]);
	} catch(exception& e) {
		cerr << e.what() << endl;
		return false;

	}
	return true;
}

void CandidateMthdFreq::setMotifSize(const int smin, const int smax)
{
	this->smin = smin;
	this->smax = smax;
}

void CandidateMthdFreq::setGraphSet(const std::vector<Graph>& gs)
{
	this->gs = &gs;
	gp.init(gs);
	for(int i = 0; i < gp.nNode; ++i)
		gp.matrix[i][i] = 0.0;
	this->nNode = gs.front().nNode;
}

std::vector<std::pair<Motif, double>> CandidateMthdFreq::getCandidantMotifs(const std::vector<Graph>& gs)
{
	setGraphSet(gs);
	vector<pair<Motif, double>> mps;
	//mps = method_enum1();
	//mps = method_node4();
	mps = method_edge2_dp();

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
			if(gp.matrix[i][j] >= pMin)
				edges.emplace_back(i, j);
	}
	return edges;
}

