#include "stdafx.h"
#include "StrategyOFG.h"

using namespace std;

void StrategyOFG::setSignature()
{
	sigPos.reserve(pgp->size());
	for(auto& sub : *pgp) {
		sigPos.push_back(genSignture(sub, pSnap));
	}
	sigNeg.reserve(pgn->size());
	for(auto& sub : *pgn) {
		sigNeg.push_back(genSignture(sub, pSnap));
	}
}

bool StrategyOFG::testMotifInSubSD(const MotifBuilder & m, const MotifSign& ms,
	const std::vector<Graph>& sub, const Signature& ss) const
{
	if(!checkSPNecessary(m, ms, ss))
		return false;
	int th = static_cast<int>(ceil(sub.size()*pSnap));
	int cnt = 0;
	for(auto&g : sub) {
		++stNumGraphChecked;
		if(g.testMotif(m)) {
			if(++cnt >= th)
				break;
		}
	}
	return cnt >= th;
}

int StrategyOFG::countMotifXSubSD(const MotifBuilder & m, const MotifSign & ms,
	const std::vector<std::vector<Graph>>& subs, const std::vector<Signature>& sigs) const
{
	int res = 0;
	for(size_t i = 0; i < subs.size(); ++i) {
		++stNumSubjectChecked;
		if(testMotifInSubSD(m, ms, subs[i], sigs[i]))
			++res;
	}
	return res;
}


StrategyOFG::Signature StrategyOFG::genSignture(
	const std::vector<Graph>& gs, const double theta)
{
	int th = static_cast<int>(ceil(theta*gs.size()));
	int n = nNode;
	//vector<sdmatrix_t> buf;
	vector<vector<vector<int>>> buf(n, vector<vector<int>>(n, vector<int>(gs.size())));
	for(size_t k = 0; k < gs.size(); ++k) {
		std::vector<std::vector<int>> m = calA2AShortestDistance(gs[k]);
		//buf.push_back(move(m));
		for(int i = 0; i < n; ++i)
			for(int j = 0; j < n; ++j)
				buf[i][j][k] = m[i][j];
	}
	Signature res(n);
	for(int i = 0; i < n; ++i)
		for(int j = 0; j < n; ++j) {
			nth_element(buf[i][j].begin(), buf[i][j].begin() + th, buf[i][j].end());
			res.sd[i][j] = buf[i][j][th - 1];
		}
	return res;
}

std::vector<std::vector<int>> StrategyOFG::calA2AShortestDistance(const Graph & g)
{
	int n = g.getnNode();
	std::vector<std::vector<int>> sd(n, vector<int>(n, 2 * n));
	for(int i = 0; i < n; ++i) {
		for(int j = 0; j < n; ++j) {
			if(g.matrix[i][j])
				sd[i][j] = 1;
		}
		//		sd[i][i] = 0;
	}
	for(int k = 0; k < n; ++k) {
		for(int i = 0; i < n; ++i)
			for(int j = 0; j < n; ++j) {
				if(sd[i][k] + sd[k][j] < sd[i][j])
					sd[i][j] = sd[i][k] + sd[k][j];
			}
	}
	return sd;
}

void StrategyOFG::updateMotifSD(MotifSign & ms, const MotifBuilder & mOld, int s, int d)
{
	vector<int> nodes;
	int n = mOld.getnNode();
	nodes.reserve(n);
	for(int i = 0; i < nNode; ++i) {
		if(mOld.containNode(i))
			nodes.push_back(i);
	}
	ms.sd[s][d] = ms.sd[d][s] = 1;
	// TODO: optimize with SPFA-like method
	for(int k = 0; k < n; ++k) {
		for(int i : nodes) {
			for(int j : nodes) {
				if(ms.sd[i][k] + ms.sd[k][j] < ms.sd[i][j])
					ms.sd[i][j] = ms.sd[i][k] + ms.sd[k][j];
			}
		}
	}
}

void StrategyOFG::calMotifSD(MotifSign & ms, const MotifBuilder & m)
{
	//init direct edges
	for(auto& e : m.edges) {
		ms.sd[e.s][e.d] = ms.sd[e.d][e.s] = 1;
	}
	//prepare nodes
	vector<int> nodes;
	int n = m.getnNode();
	nodes.reserve(n);
	for(int i = 0; i < nNode; ++i) {
		if(m.containNode(i))
			nodes.push_back(i);
	}
	//update
	for(int k = 0; k < n; ++k) {
		for(int i : nodes) {
			for(int j : nodes) {
				if(ms.sd[i][k] + ms.sd[k][j] < ms.sd[i][j])
					ms.sd[i][j] = ms.sd[i][k] + ms.sd[k][j];
			}
		}
	}
}

bool StrategyOFG::checkSPNecessary(const MotifBuilder& m, const MotifSign & ms, const Signature & ss) const
{
	// all edges should: sdis(e;g) <= sdis(e;m)
	for(const Edge& e : m.edges) {
		if(ms.sd[e.s][e.d] < ss[e.s][e.d])
			return false;
	}
	return true;
}
