#include "stdafx.h"
#include "CandidateMthdFreq.h"

using namespace std;

// ------------------- method edge 1 --------------------

std::vector<std::pair<Motif, double>> CandidateMthdFreq::method_edge1_bfs()
{
	vector<pair<Motif, double>> mps;
	vector<Edge> edges = getEdges(gp);
	vector<pair<MotifBuilder, double>> last;
	mps.reserve(3 * edges.size());
	last.reserve(3 * edges.size());
	for(const Edge& e : edges) {
		MotifBuilder m;
		m.addEdge(e.s, e.d);
		double p = gp.matrix[e.s][e.d];
		last.emplace_back(move(m), p);
	}
	for(int s = 2; s <= smax; ++s) {
		vector<pair<MotifBuilder, double>> t = _edge1_bfs(last, edges);
		sort(t.begin(), t.end());
		auto itend = unique(t.begin(), t.end());
		cout << t.end() - itend << " / " << t.size() << " redundant motifs of size " << s << endl;
		t.erase(itend, t.end());
		if(t.empty())
			break;
		if(s-1 >= smin) {
			for(auto& mbp : last)
				mps.emplace_back(mbp.first.toMotif(), mbp.second);
		}
		last = move(t);
	}
	for(auto& mbp : last)
		mps.emplace_back(mbp.first.toMotif(), mbp.second);

	return mps;
}

std::vector<std::pair<MotifBuilder, double>> CandidateMthdFreq::_edge1_bfs(
	const std::vector<pair<MotifBuilder, double>>& last, const std::vector<Edge>& edges)
{
	std::vector<std::pair<MotifBuilder, double>> res;
	for(const auto& mp : last) {
		const auto& m = mp.first;
		for(const Edge&e : edges) {
			// just contain one node of new edge 
			// otherwise don't contain the new edge
			if((m.containNode(e.s) || m.containNode(e.d)) && !m.containEdge(e.s, e.d)) {
				// prune, but lost some valid motifs:
				//if(e < m.lastEdge())
				//	continue;
				MotifBuilder t(m);
				t.addEdge(e.s, e.d);
				double p = probOfMotif(t, *gs);
				if(p >= pMin) {
					res.emplace_back(move(t), p);
				}
			}
		}
	}
	return res;
}

// ------------------- method edge 2 --------------------

std::vector<std::pair<Motif, double>> CandidateMthdFreq::method_edge2_dp()
{
	vector<pair<Motif, double>> mps;
	vector<pair<MotifBuilder, double>> open;
	open.emplace_back(MotifBuilder(), 1.0);

	vector<Edge> edges = getEdges(gp);
	// maintain: mps: maxized result (s==smax)
	//			open: expandable result (0<=s<=smax-1)
	for(const Edge& e : edges) {
		vector<pair<MotifBuilder, double>> t = _edge2_dp(open, e);
		for(auto& mp : t) {
			int n = mp.first.getnEdge();
			if(n == smax) {
				mps.emplace_back(mp.first.toMotif(), mp.second);
			} else {
				open.push_back(move(mp));
			}
		}
	}

	for(auto& mp: open) {
		if(mp.first.getnEdge() >= smin)
			mps.emplace_back(mp.first.toMotif(), mp.second);
	}
	return mps;
}

std::vector<std::pair<MotifBuilder, double>> CandidateMthdFreq::_edge2_dp(
	const std::vector<std::pair<MotifBuilder, double>>& last, const Edge& e)
{
	vector<std::pair<MotifBuilder, double>> res;
	for(const auto& mp : last) {
		if(mp.first.getnEdge() >= smax)
			continue;
		if(mp.first.empty() || mp.first.containNode(e.s) || mp.first.containNode(e.d)) {
			MotifBuilder t(mp.first);
			t.addEdge(e.s, e.d);
			double p = probOfMotif(t, *gs);
			if(p >= pMin) {
				res.push_back(make_pair(move(t), p));
			}
		}
	}
	return res;
}

