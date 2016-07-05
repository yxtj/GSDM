#include "stdafx.h"
#include "CandidateMthdFreq.h"

using namespace std;

// ------------------- method edge 1 --------------------

std::vector<std::pair<Motif, double>> CandidateMthdFreq::method_edge1_bfs()
{
	vector<pair<Motif, double>> mps;
	vector<Edge> edges = getEdges(gp);
	vector<pair<Motif, double>> last;
	mps.reserve(3 * edges.size());
	last.reserve(3 * edges.size());
	for(const Edge& e : edges) {
		Motif m;
		m.addEdge(e.s, e.d);
		double p = gp.matrix[e.s][e.d];
		last.push_back(make_pair(move(m), p));
	}
	for(int s = 2; s <= smax; ++s) {
		vector<pair<Motif, double>> t = _edge1_bfs(last, edges);
		sort(t.begin(), t.end());
		auto itend = unique(t.begin(), t.end());
		cout << t.end() - itend << " / " << t.size() << " redundant motifs of size " << s << endl;
		t.erase(itend, t.end());
		if(t.empty())
			break;

		if(s-1 >= smin) {
			// move the motifs with size (s-1) to the result set, to avoid copy
			move(last.begin(), last.end(), back_inserter(mps));
		}
		last = move(t);
	}
	move(last.begin(), last.end(), back_inserter(mps));

	return mps;
}

std::vector<std::pair<Motif, double>> CandidateMthdFreq::_edge1_bfs(
	const std::vector<pair<Motif, double>>& last, const std::vector<Edge>& edges)
{
	std::vector<std::pair<Motif, double>> res;
	for(const auto& mp : last) {
		const Motif& m = mp.first;
		for(const Edge&e : edges) {
			// just contain one node of new edge 
			// otherwise don't contain the new edge
			bool b1 = m.containNode(e.s), b2 = m.containNode(e.d);
			if(b1 ^ b2 || ((b1&b2) && !m.containEdge(e.s, e.d))) {
				// prune, but lost some valid motifs:
				//if(e < m.lastEdge())
				//	continue;
				Motif t(m);
				t.addEdge(e.s, e.d);
				double p = probOfMotif(t, *gs);
				if(p >= pMin) {
					res.push_back(make_pair(move(t), p));
				}
			}
		}
	}
	return res;
}

// ------------------- method edge 2 --------------------

std::vector<std::pair<Motif, double>> CandidateMthdFreq::method_edge2_dp()
{
	vector<pair<Motif, double>> mps, open;
	const pair<Motif, double> dummy(Motif(), 1.0);
	open.push_back(dummy);

	vector<Edge> edges = getEdges(gp);
	// this iteration: mps: s==smax; open: expandable result (0<=s<=smax-1)
	for(const Edge& e : edges) {
		vector<pair<Motif, double>> t = _edge2_dp(open, e);
		for(auto& mp : t) {
			int n = mp.first.getnEdge();
			if(n == smax) {
				mps.push_back(move(mp));
			} else {
				open.push_back(move(mp));
			}
		}
	}

	for(auto& mp: open) {
		if(mp.first.getnEdge() >= smin)
			mps.push_back(move(mp));
	}
	return mps;
}

std::vector<std::pair<Motif, double>> CandidateMthdFreq::_edge2_dp(
	const std::vector<std::pair<Motif, double>>& last, const Edge& e)
{
	vector<std::pair<Motif, double>> res;
	for(const auto& mp : last) {
		if(mp.first.getnEdge() >= smax)
			continue;
		if(mp.first.empty() || mp.first.containNode(e.s) || mp.first.containNode(e.d)) {
			Motif t(mp.first);
			t.addEdge(e.s, e.d);
			double p = probOfMotif(t, *gs);
			if(p >= pMin) {
				res.push_back(make_pair(move(t), p));
			}
		}
	}
	return res;
}

