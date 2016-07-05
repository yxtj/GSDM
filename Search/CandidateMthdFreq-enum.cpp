#include "stdafx.h"
#include "CandidateMthdFreq.h"

using namespace std;

std::vector<std::pair<Motif, double>> CandidateMthdFreq::method_enum1() {
	vector<Edge> edges = getEdges(gp);
	pair<Motif, double> dummy;
	dummy.second = 1.0;
	vector<pair<Motif, double>> mps = _enum1(0, dummy, edges);
	sort(mps.begin(), mps.end());
	return mps;
}

std::vector<std::pair<Motif, double>> CandidateMthdFreq::_enum1(
	const unsigned p, const std::pair<Motif, double>& curr,
	const std::vector<Edge>& edges)
{
	std::vector<std::pair<Motif, double>> res;
	if(curr.first.getnEdge() >= smax) {
		if(curr.first.connected())
			res.push_back(curr);
		return res;
	} else if(p >= edges.size()) {
		if(curr.first.getnEdge() >= smin && curr.first.connected())
			res.push_back(curr);
		return res;
	}
	res = _enum1(p + 1, curr, edges);

	pair<Motif, double> mp(curr);
	mp.first.addEdge(edges[p].s, edges[p].d);
	mp.second = probOfMotif(mp.first, *gs);
	if(mp.second >= pMin) {
		auto t = _enum1(p + 1, mp, edges);
		move(t.begin(), t.end(), back_inserter(res));
	}
	return res;
}

