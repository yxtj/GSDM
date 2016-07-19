#include "stdafx.h"
#include "CandidateMethod.h"

using namespace std;

double CandidateMethod::probOfMotif(const Motif & m, const std::vector<Graph>& gs)
{
	int cnt = 0;
	for(const Graph& g : gs) {
		if(g.testMotif(m))
			++cnt;
	}
	return static_cast<double>(cnt) / gs.size();
}

double CandidateMethod::probOfMotif(const MotifBuilder & m, const std::vector<Graph>& gs)
{
	int cnt = 0;
	for(const Graph& g : gs) {
		if(g.testMotif(m))
			++cnt;
	}
	return static_cast<double>(cnt) / gs.size();
}

bool CandidateMethod::testProbOfMotifGE(const Motif & m, const std::vector<Graph>& gs, const double pMin)
{
	int nMin= static_cast<int>(ceil(gs.size()*pMin));
	int cnt = 0;
	for(auto it = gs.begin(); cnt < nMin && it != gs.end(); ++it) {
		if(it->testMotif(m))
			++cnt;
	}
	return cnt >= nMin;
}
bool CandidateMethod::testProbOfMotifGE(const MotifBuilder & m, const std::vector<Graph>& gs, const double pMin)
{
	int nMin = static_cast<int>(ceil(gs.size()*pMin));
	int cnt = 0;
	for(auto it = gs.begin(); cnt < nMin && it != gs.end(); ++it) {
		if(it->testMotif(m))
			++cnt;
	}
	return cnt >= nMin;
}

bool CandidateMethod::testProbOfMotifLE(const Motif & m, const std::vector<Graph>& gs, const double pMax)
{
	int nMax = static_cast<int>(floor(gs.size()*pMax));
	int cnt = 0;
	for(auto it = gs.begin(); cnt <= nMax && it != gs.end(); ++it) {
		if(it->testMotif(m))
			++cnt;
	}
	return cnt <= nMax;
}
bool CandidateMethod::testProbOfMotifLE(const MotifBuilder & m, const std::vector<Graph>& gs, const double pMax)
{
	int nMax = static_cast<int>(floor(gs.size()*pMax));
	int cnt = 0;
	for(auto it = gs.begin(); cnt <= nMax && it != gs.end(); ++it) {
		if(it->testMotif(m))
			++cnt;
	}
	return cnt <= nMax;
}

std::vector<Motif> CandidateMethod::toMotif(const std::vector<MotifBuilder>& mbs)
{
	std::vector<Motif> res;
	res.reserve(mbs.size());
	for(auto & mb : mbs) {
		res.push_back(mb.toMotif());
	}
	return res;
}
