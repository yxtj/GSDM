#include "stdafx.h"
#include "CandidateMthdFreq.h"

using namespace std;

std::vector<std::pair<Motif, double>> CandidateMthdFreq::method_edge1()
{
	vector<pair<Motif, double>> mps;
	pair<Motif, double> dummy;
	dummy.second = 1.0;
	for(int s = 0; s < nNode; ++s) {
		auto t = _edge1(dummy, s);
		mps.reserve(mps.size() + t.size());
		move(t.begin(), t.end(), back_inserter(mps));
	}
	sort(mps.begin(), mps.end());
	auto itend = unique(mps.begin(), mps.end());
	cout << mps.end() - itend << " / " << mps.size() << " redundant motifs " << endl;
	mps.erase(itend, mps.end());
	return mps;
}

std::vector<std::pair<Motif, double>> CandidateMthdFreq::_edge1(
	const std::pair<Motif, double>& curr, const int expNode)
{
	std::vector<std::pair<Motif, double>> res;
	return res;
}

