#include "stdafx.h"
#include "CandidateMthdFreq.h"

using namespace std;


// ------------------- method 2 -------------------------

class _DfsEdgeComSearcher {
	const std::pair<Motif, double>& startMP;
	const int expNode;
	const std::vector<Graph>* gs;
	const GraphProb& gp;
	const int nNode;
	const double pMin;
	const int smax;

	vector<int> neighbors;
public:
	_DfsEdgeComSearcher(const std::pair<Motif, double>& mp, const int expNode,
		const std::vector<Graph>* gs, const GraphProb& gp,
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
		double prob = CandidateMethod::probOfMotif(mt, *gs);
		if(prob >= pMin) {
			vector<int> nodes = get<2>(state);
			nodes.push_back(newNode);
			auto t = search(p + 1, make_tuple(move(mt), prob, nodes));
			res.reserve(res.size() + t.size());
			res.insert(res.end(), make_move_iterator(t.begin()), make_move_iterator(t.end()));
		}
	}
	return res;
}

std::vector<std::pair<Motif, double>> CandidateMthdFreq::method_node2_layer()
{
	vector<pair<Motif, double>> mps;
	pair<Motif, double> dummy;
	dummy.second = 1.0;
	for(int s = 0; s < nNode; ++s) {
		_node2_layer(mps, dummy, s);
	}
	sort(mps.begin(), mps.end());
	auto itend = unique(mps.begin(), mps.end());
	cout << mps.end() - itend << " / " << mps.size() << " redundant motifs " << endl;
	mps.erase(itend, mps.end());
	return mps;
}

void CandidateMthdFreq::_node2_layer(std::vector<std::pair<Motif, double>>& res,
	const std::pair<Motif, double>& curr, const int expNode)
{
	int nEdge = curr.first.getnEdge();
//	if(nEdge >= smin) {
//		closed.push_back(curr);
//	}
	if(nEdge >= smin /*&& nEdge <= smax*/) {
		res.push_back(curr);
		if(nEdge >= smax)
			return;
	}
	// enumerate all surrounding edge combinations of current motif at current expanding node
	_DfsEdgeComSearcher openSetSearcher(curr, expNode, gs, gp, nNode, par->pMin, smax);
	vector<tuple<Motif, double, vector<int>>> edgeCom =
		openSetSearcher.search(0, make_tuple(curr.first, curr.second, vector<int>()));
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
				double prob = probOfMotif(m, *gs);
				if(prob >= par->pMin)
					_node2_layer(res, make_pair(move(m), prob), newNode);
			}
		}
	}
}

// ------------------- method 3 -------------------------

std::vector<std::pair<Motif, double>> CandidateMthdFreq::method_node3()
{
	vector<pair<Motif, double>> mps;
	pair<Motif, double> dummy;
	dummy.second = 1.0;

	vector<pair<Motif, double>> open;
	for(int s = 0; s < nNode; ++s) {
		_node3(mps, open, dummy, s);
	}
	for(size_t i = 0; i < open.size(); ++i) {
		if(open[i].first.getnEdge() >= smin)
			mps.push_back(move(open[i]));
	}

	sort(mps.begin(), mps.end());
	auto itend = unique(mps.begin(), mps.end());
	cout << mps.end() - itend << " / " << mps.size() << " redundant motifs " << endl;
	mps.erase(itend, mps.end());

	return mps;
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
void CandidateMthdFreq::_node3(std::vector<std::pair<Motif, double>>& closed,
	std::vector<std::pair<Motif, double>>& open,
	const std::pair<Motif, double>& rootMP, const int expNode)
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
	validNeighbors.reserve(nNode / 2 - rootMP.first.getnNode());
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
				_node3(closed, open, make_pair(move(m), p), newNode);
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
				double p = probOfMotif(m, *gs);
				if(p >= par->pMin) {
					_node3(closed, open, make_pair(move(m), p), newNode);
				}
			} // if condition
		} // for open
	}
}

// ------------------- method 4 -------------------------

std::vector<std::pair<Motif, double>> CandidateMthdFreq::method_node4()
{
	vector<pair<Motif, double>> mps;
	pair<Motif, double> dummy;
	dummy.second = 1.0;
	for(int s = 0; s < nNode; ++s) {
		auto t = _node4(dummy, s);
		//mps.reserve(mps.size() + t.size());
		for(auto& mp : t) {
			if(mp.first.getnEdge() >= smin)
				mps.push_back(move(mp));
		}
		//move(t.begin(), t.end(), back_inserter(mps));
	}
	sort(mps.begin(), mps.end());
	auto itend = unique(mps.begin(), mps.end());
	cout << mps.end() - itend << " / " << mps.size() << " redundant motifs " << endl;
	mps.erase(itend, mps.end());
	return mps;
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
std::vector<std::pair<Motif, double>> CandidateMthdFreq::_node4(
	const std::pair<Motif, double>& curr, const int expNode)
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
				&& (mp.first.getnEdge() == 0 || mp.first.lastEdge() < Edge{ expNode, i })) {
				Motif m(mp.first);
				m.addEdge(expNode, i);
				double p = probOfMotif(m, *gs);
				if(p >= par->pMin) {
					auto next = make_pair(move(m), p);
					auto t = _node4(next, i);
					res.reserve(res.size() + t.size());
					res.insert(res.end(), make_move_iterator(t.begin()), make_move_iterator(t.end()));
				}
			}
		}
	}
	return res;
}

