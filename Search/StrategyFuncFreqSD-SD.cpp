#include "stdafx.h"
#include "StrategyFuncFreqSD.h"
#include "CandidateMethodFactory.h"
#include "Option.h"
#include "Network.h"
#include "../util/Timer.h"

using namespace std;


void StrategyFuncFreqSD::setSignature()
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

bool StrategyFuncFreqSD::testMotifSP(const MotifBuilder & m, const MotifSign& ms,
	const std::vector<Graph>& sub, const Signature& ss) const
{
	if(!checkSPNecessary(m, ms, ss))
		return false;
	int th = static_cast<int>(ceil(sub.size()*pSnap));
	int cnt = 0;
	for(auto&g : sub) {
		if(g.testMotif(m)) {
			++stNumGraphChecked;
			if(++cnt >= th)
				break;
		}
	}
	return cnt >= th;
}

int StrategyFuncFreqSD::countMotifSP(const MotifBuilder & m, const MotifSign & ms,
	const std::vector<std::vector<Graph>>& subs, const std::vector<Signature>& sigs) const
{
	int res = 0;
	for(size_t i = 0; i < subs.size(); ++i) {
		if(testMotifSP(m, ms, subs[i], sigs[i]))
			++res;
	}
	return res;
}


StrategyFuncFreqSD::Signature StrategyFuncFreqSD::genSignture(
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

std::vector<std::vector<int>> StrategyFuncFreqSD::calA2AShortestDistance(const Graph & g)
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

void StrategyFuncFreqSD::updateMotifSD(MotifSign & ms, const MotifBuilder & mOld, int s, int d)
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

void StrategyFuncFreqSD::calMotifSD(MotifSign & ms, const MotifBuilder & m)
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

bool StrategyFuncFreqSD::checkSPNecessary(const MotifBuilder& m, const MotifSign & ms, const Signature & ss) const
{
	// all edges should: sdis(e;g) <= sdis(e;m)
	for(const Edge& e : m.edges) {
		if(ms.sd[e.s][e.d] < ss[e.s][e.d])
			return false;
	}
	return true;
}

std::vector<MotifBuilder> StrategyFuncFreqSD::sortUpNewLayer(std::vector<MotifBuilder>& layer)
{
	if(flagNetworkPrune)
		return pruneWithNumberOfParents(layer);
	else
		return removeDuplicate(layer);
}

std::vector<MotifBuilder> StrategyFuncFreqSD::removeDuplicate(std::vector<MotifBuilder>& layer)
{
	sort(layer.begin(), layer.end());
	auto itend = unique(layer.begin(), layer.end());
	layer.erase(itend, layer.end());
	return layer;
}

int StrategyFuncFreqSD::quickEstimiateNumberOfParents(const Motif & m)
{
	unordered_set<int> uniqueNodes, duplicateNodes;
	auto fun = [&](const int n) {
		if(duplicateNodes.find(n) == duplicateNodes.end()) {
			auto it = uniqueNodes.find(n);
			if(it == uniqueNodes.end()) {
				uniqueNodes.insert(n);
			} else {
				uniqueNodes.erase(it);
				duplicateNodes.insert(n);
			}
		}
	};
	for(auto& e : m.edges) {
		fun(e.s);
		fun(e.d);
	}
	return uniqueNodes.size();
}

int StrategyFuncFreqSD::quickEstimiateNumberOfParents(const MotifBuilder & m)
{
	int cnt = 0;
	for(auto& p : m.nodes) {
		if(p.second == 1)
			++cnt;
	}
	return cnt;
}

std::vector<MotifBuilder> StrategyFuncFreqSD::pruneWithNumberOfParents(std::vector<MotifBuilder>& mbs)
{
	if(mbs.empty())
		return mbs;
	sort(mbs.begin(), mbs.end());
	auto p = mbs.begin(); // point to the processing position
	auto pGroup= p; // point to the first of current group
	auto pRes = p; // point to an undetermined position
	auto pEnd = mbs.end();
	int cnt = 1;
	while(++p != pEnd) {
		if(!(*pGroup == *p)) { // new motif
			int nParentMin = quickEstimiateNumberOfParents(*pGroup);
			if(nParentMin <= p-pGroup) {
				// all (all found) its parents are qualified
				if(pRes != pGroup)
					*pRes = move(*pGroup);
				++pRes;
			}
			pGroup = p;
		}
		//if(!(*pRes == *p) && ++pRes != p) {
		//	*pRes = std::move(*p);
		//}
	}
	mbs.erase(pRes, pEnd);
	return mbs;
}

