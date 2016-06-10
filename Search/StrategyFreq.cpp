#include "stdafx.h"
#include "StrategyFreq.h"

#include <iostream>

using namespace std;

const std::string StrategyFreq::name("Freq");

StrategyFreq::StrategyFreq()
{
}

std::vector<std::pair<Motif, double>> StrategyFreq::getCandidantMotifs(const std::vector<Graph>& gs,
	const int smin, const int smax, const SearchStrategyPara& par)
{
	this->smin = smin;
	this->smax = smax;
	const StrategyFreqPara& param = static_cast<const StrategyFreqPara&>(par);
	this->par = &param;
	GraphProb gp(gs);
	this->nNode = gp.nNode;
//	cout << "test in StrategyFreq::motifOnIndTopK" << endl;
//	cout << param.pMin << endl;

	
	vector<pair<Motif, double>> mps;
	pair<Motif, double> dummy;
	dummy.second = 1.0;
/*	for(int s = 0; s < nNode; ++s) {
//		dfsMotif(mps, s, gs, param, gp);
		dfsMotif2(mps, dummy, s, gs, gp);
	}
*/
/*	vector<pair<Motif, double>> open;
	for(int s = 0; s < nNode; ++s) {
		dfsMotif3(mps, open, dummy, s, gs, gp);
	}
	for(size_t i = 0; i < open.size(); ++i) {
		if(open[i].first.getnEdge() >= smin)
			mps.push_back(move(open[i]));
	}
*/
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

// Return all the qualified edge combinations of the expPoints
// require: expPoints to be sorted
vector<vector<Edge>> dfsEdgeCom(vector<Edge>& curr, const int pExp, const int pDest,
	const Motif& m, const double prob, const vector<int>& expPoints,
	const GraphProb& gp, const double pMin, const int smax, const int nNode)
{
	if(m.getnEdge() + curr.size() >= smax) {
		vector<vector<Edge>> res;
		res.push_back(curr);
		return res;
	} else if(pExp == expPoints.size() && pDest == nNode) {
		vector<vector<Edge>> res;
		res.push_back(curr);
		return res;
	}

	const int edgeLeft = smax - m.getnEdge();
	const double probReq = pMin / prob;
	vector<vector<Edge>> res;
	
	// move to next acceptable edge
	int pDestNew = pDest + 1;
	int pExpNew = pExp;
	while(pExpNew < expPoints.size()) {
		while(pDestNew < nNode && gp.matrix[expPoints[pExpNew]][pDestNew] < probReq) {
			++pDestNew;
		}
		if(pDestNew == nNode) {
			++pExpNew;
			// only expand to nodes with greater id
			pDestNew = pExpNew + 1;
		} else {
			break;
		}
	}

	vector<vector<Edge>> t;
	t = dfsEdgeCom(curr, pExpNew, pDestNew, m, prob, expPoints, gp, pMin, smax, nNode);
	for(auto& v : t) {
		res.push_back(move(v));
	}
	if(pExpNew >= expPoints.size())
		return res;

	curr.push_back(Edge{ expPoints[pExp],pDest });
	double p = gp.matrix[expPoints[pExpNew]][pDestNew];
	dfsEdgeCom(curr, pExpNew, pDestNew, m, prob*p, expPoints, gp, pMin, smax, nNode);

	return res;
}

// only explore edges with greater lexicographical order
// edges are compared with pair (s,d) where s<d
// So only explore to nodes with greater id
// Prune: 
//	1, maximum probability ( <= that motif in indenpendent uncertain graph)
//	2, expand to nodes with greater id
void StrategyFreq::dfsMotif(std::vector<std::pair<Motif, double>>& mps, const int expNode,
	const std::vector<Graph>& gs, const StrategyFreqPara& par, const GraphProb& gp)
{
	for(int i = expNode + 1; i < nNode; ++i) {
		// rough test
		double p = gp.matrix[expNode][i];
		if(p >= par.pMin) {
			Motif t;
			t.addEdge(expNode, i);
			// fine test
			mps.emplace_back(t, p);
		}
	}
	for(size_t _i = 0; _i <  mps.size(); ++_i) {
		pair<Motif, double> mp = mps[_i]; // make a copy because insert action may invalid current value
		//for(const pair<Motif, double>& mp : mps) {
		if(mp.first.getnEdge() == smax)
			continue;
		if(!mp.first.containNode(expNode))
			continue;
		for(int i = expNode + 1; i < nNode; ++i) {
			// rough test
			if(!mp.first.containEdge(expNode, i)
				&& mp.second*gp.matrix[expNode][i] >= par.pMin) 
			{
				Motif t(mp.first);
				t.addEdge(expNode, i);
				// fine test
				double p = probOfMotif(t, gs);
				if(p >= par.pMin) {
					mps.emplace_back(t, p);
					//dfsMotif(mps, i, gs, par);
				}
			}
		}
	}

}

class _DfsEdgeComSearcher {
	const std::pair<Motif, double>& startMP;
	const int expNode;
	const std::vector<Graph>& gs;
	const GraphProb& gp;
	const int nNode; 
	const double pMin;
	const int smax;
	
	vector<int> neighbors;
public:
	_DfsEdgeComSearcher(const std::pair<Motif, double>& mp, const int expNode,
		const std::vector<Graph>& gs, const GraphProb& gp,
		const int nNode, const double pMin, const int smax)
		:startMP(mp), expNode(expNode), gs(gs), gp(gp), nNode(nNode), pMin(pMin), smax(smax)
	{
		neighbors.reserve(nNode - startMP.first.getnNode());
		for(int i = 0; i < nNode; ++i) {
			if(startMP.second*gp.matrix[expNode][i] >= pMin
				&& !startMP.first.containEdge(expNode, i))
				neighbors.push_back(i);
		}
	}

	// get valid combinations of expanding edges (except the empty set)
	// return <new motif, prob. of new motif, new nodes>
	vector<tuple<Motif, double, vector<int>>> search(const int p, const tuple<Motif, double, vector<int>>& state);
};

vector<tuple<Motif, double, vector<int>>> _DfsEdgeComSearcher::search(
	const int p, const tuple<Motif, double, vector<int>>& state)
{
	vector<tuple<Motif, double, vector<int>>> res;
	const Motif& stateM = get<0>(state);
	if(stateM.getnEdge() >= smax) {
		if(stateM.getnEdge() == smax)
			res.push_back(state);
		return res;
	} else if(p == neighbors.size()) {
		if(stateM.getnEdge() != 0)
			res.push_back(state);
		return res;
	}
	// do not add current edge
	res = search(p + 1, state);
	// add current edge
	const double& stateP = get<1>(state);
	int newNode = neighbors[p];
	if(stateP*gp.matrix[expNode][newNode] >= pMin &&
		(!stateM.containNode(newNode) || !stateM.containEdge(expNode, newNode))) {
		Motif mt(stateM);
		mt.addEdge(expNode, newNode);
		double prob = SearchStrategy::probOfMotif(mt, gs);
		if(prob >= pMin) {
			vector<int> nodes = get<2>(state);
			nodes.push_back(newNode);
			auto t=search(p + 1, make_tuple(move(mt), prob, nodes));
			res.reserve(res.size() + t.size());
			res.insert(res.end(), make_move_iterator(t.begin()), make_move_iterator(t.end()));
		}
	}
	return res;
}

void StrategyFreq::dfsMotif2(std::vector<std::pair<Motif, double>>& res, 
	const std::pair<Motif, double>& curr, const int expNode,
	const std::vector<Graph>& gs, const GraphProb& gp)
{
	int nEdge = curr.first.getnEdge();
//	if(nEdge >= smin) {
//		closed.push_back(curr);
//	}
	if(nEdge >= smin /*&& nEdge <= smax*/) {
		res.push_back(curr);
		if(nEdge>=smax)
			return;
	}
	// enumerate all surrounding edge combinations of current motif at current expanding node
	_DfsEdgeComSearcher openSetSearcher(curr, expNode, gs, gp, nNode, par->pMin, smax);
	vector<tuple<Motif, double, vector<int>>> edgeCom = 
		openSetSearcher.search(0, make_tuple(curr.first,curr.second,vector<int>()));
	// expand to all these new edge sets
	for(auto& mpn : edgeCom) {
		Motif& edgeComM = get<0>(mpn);
		double edgeComP = get<1>(mpn);
		const vector<int>& edgeComN = get<2>(mpn);
		if(edgeComM.getnEdge() == smax) {
			res.push_back(make_pair(move(edgeComM), edgeComP));
			continue;
		} else if(edgeComM.getnEdge() >= smin) {
			res.push_back(make_pair(edgeComM, edgeComP));
		} // openSetSearcher ensures the case "edgeComM.getnEdge()>max" is impossible
		// expand on every new node(s)
		// pre-condition: m.getnEdge() is in the range [0, smax-1]
		for(size_t i = 0; i < edgeComN.size(); ++i) {
			int newNode = edgeComN[i];
			if(edgeComP*gp.matrix[expNode][newNode] >= par->pMin
				&& (edgeComM.getnEdge() == 0 || *edgeComM.edges.rbegin() < Edge{ expNode, newNode }) //gSpan's idea (lexicographical order on edge)
				&& !edgeComM.containEdge(expNode, newNode)) {
				Motif m(edgeComM);
				m.addEdge(expNode, newNode);
				double prob = probOfMotif(m, gs);
				if(prob >= par->pMin)
					dfsMotif2(res, make_pair(move(m), prob), newNode, gs, gp);
			}
		}
	}
}

/************************************************************************/
/* 
return:
	the new motifs expanded from the common root motif at expNode
condition: 
	closed: motifs reached the size of smax
	open: motifs contains the common rootMotif and with a size smaller than smax
	rootMP: the common root motif and its probability
	expNode: the node to expand from, contained by the common root motif

*/
/************************************************************************/
void StrategyFreq::dfsMotif3(std::vector<std::pair<Motif, double>>& closed,
	std::vector<std::pair<Motif, double>>& open,
	const std::pair<Motif, double>& rootMP, const int expNode,
	const std::vector<Graph>& gs, const GraphProb& gp)
{
	int nEdge = rootMP.first.getnEdge();
	if(nEdge >= smax) {
		closed.push_back(rootMP);
		return;
	}
	if(nEdge != 0)
		open.push_back(rootMP);
	// generate a set of valid neighbor nodes based on current root
	// to reduce the checking operations in the next step
	vector<int> validNeighbors;
	validNeighbors.reserve(nNode/2 - rootMP.first.getnNode());
	for(int i = 0; i < nNode; ++i) {
		if(rootMP.second*gp.matrix[expNode][i] >= par->pMin
			&& !rootMP.first.containEdge(expNode, i)) {
			validNeighbors.push_back(i);
		}
	}
	// expand to each valid neighbor nodes (from each motifs in the open set)
	for(int& newNode : validNeighbors) {
		// expand from all the motifs in open set & update the open set
		size_t currOpenSize = open.size();
		if(currOpenSize == 0) {
			Motif m;
			m.addEdge(expNode, newNode);
			double p = gp.matrix[expNode][newNode];
			if(p >= par->pMin) {
				dfsMotif3(closed, open, make_pair(move(m), p), newNode, gs, gp);
			}
			continue;
		}
		for(size_t i = 0; i < currOpenSize; ++i) {
			pair<Motif, double> mp = open[i];
			if(mp.first.containNode(expNode)
				&& min(mp.second, gp.matrix[expNode][newNode]) >= par->pMin
				&& (mp.first.getnEdge() == 0 || *mp.first.edges.rbegin() < Edge{ expNode, newNode }))
				//the last line is gSpan condition, which ensures "!mp.first.containEdge(expNode, newNode)"
			{
				Motif m(mp.first);
				m.addEdge(expNode, newNode);
				double p = probOfMotif(m, gs);
				if(p >= par->pMin) {
					dfsMotif3(closed, open, make_pair(move(m), p), newNode, gs, gp);
				}
			} // if condition
		} // for open
	}
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
std::vector<std::pair<Motif, double>> StrategyFreq::dfsMotif4(
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
		// pre-prune: only expand to frequent edges
		if(gp.matrix[expNode][i] < par->pMin && !curr.first.containEdge(expNode, i))
			continue;
		size_t currResSize = res.size();
		for(size_t j = 0; j < currResSize; ++j) {
			auto& mp = res[j];
			if(mp.first.getnEdge() < smax
				&& (mp.first.getnEdge()==0 || mp.first.lastEdge() < Edge{ expNode, i })) {
				Motif m(mp.first);
				m.addEdge(expNode, i);
				double p = probOfMotif(m, gs);
				if(p >= par->pMin) {
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
std::vector<std::pair<Motif, double>> StrategyFreq::dfsMotif5(const std::pair<Motif, double>& curr, const int expNode, const std::vector<Graph>& gs, const GraphProb & gp)
{
	return std::vector<std::pair<Motif, double>>();
}



// ---------------------------- Parameter Class: ----------------------------

void StrategyFreqPara::construct(
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
