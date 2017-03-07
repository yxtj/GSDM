#include "stdafx.h"
#include "StrategyOFG.h"
#include "../util/Timer.h"

using namespace std;


std::vector<Motif> StrategyOFG::method_edge1_bfs()
{
	cout << "Phase 1 (prepare edges)" << endl;
	//vector<Edge> edges = initialCandidateEdges();
	vector<pair<Edge, double>> edges = getExistedEdges(*pdp);
	cout << "  # of edges: " << edges.size() << endl;
	//	vector<pair<MotifBuilder, double>> last;
	vector<MotifBuilder> last;
	last.reserve(3 * edges.size());
	for(const pair<Edge, double>& e : edges) {
		MotifBuilder m;
		m.addEdge(e.first.s, e.first.d);
		//		double p = gp.matrix[e.s][e.d];
		//		last.emplace_back(move(m), p);
		last.push_back(move(m));
	}
	if(last.empty()) {
		return vector<Motif>();
	}

	cout << "Phase 2 (testing motifs layer by layer)" << endl;
	cout << "  level (x): (x) ms, on (x) motifs\t"
		"generate: (# valid motifs)/(# total motifs)\t"
		"k-th score: (score of the worst in Top-K)\t"
		"CE rest: (rest candidate edges), remove (# removed by connection) + (# removed by bound)" << endl;
	TopKHolder<Motif, double> holder(k);
	int smax = edges.size()*(edges.size() - 1) / 2;
	for(int s = 2; s <= smax; ++s) {
		Timer timer;
		size_t numLast, numTotal, numUnique, numRmvEdgeConnect, numRmvEdgeBound;
		numLast = last.size();
		vector<bool> usedEdges(edges.size(), false);
		map<MotifBuilder, int> t = _edge1_bfs(holder, last, edges, usedEdges);
		tie(last, numTotal) = sortUpNewLayer(t);
		numUnique = last.size();

		//int nRmvEdge = (this->*maintainDCESConnected)(edges, usedEdges);
		//nRmvEdge += (this->*maintainDCESBound)(edges, holder.lastScore());
		if(flagDCESConnected)
			numRmvEdgeConnect = maintainDCESConnected(edges, usedEdges);
		if(flagDCESBound)
			numRmvEdgeBound = maintainDCESBound(edges, holder.lastScore());
		smax = edges.size()*(edges.size() - 1) / 2;
		auto _time_ms = timer.elapseMS();
		cout << "  level " << s - 1 << " : " << _time_ms << " ms, on " << numLast << " motifs."
			<< "\tgenerate: " << numUnique << " / " << numTotal << "."
			<< "\tk-th score: " << holder.lastScore();
		if(flagDCESConnected || flagDCESBound)
			cout << "\tCE rest: " << edges.size() << ", remove: " << numRmvEdgeConnect << " + " << numRmvEdgeBound;
		cout << endl;
		if(last.empty())
			break;
	}

	cout << "Phase 3 (output)" << endl;
	if(flagOutputScore) {
		ofstream fout(pathOutputScore);
		for(auto& p : holder.data) {
			fout << p.second << "\t" << p.first << "\n";
		}
	}
	return holder.getResultMove();
}

std::map<MotifBuilder, int> StrategyOFG::_edge1_bfs(
	TopKHolder<Motif, double>& holder, const std::vector<MotifBuilder>& last,
	std::vector<std::pair<Edge, double>>& edges, std::vector<bool>& usedEdge)
{
	int layer = last.front().getnEdge();
	std::map<MotifBuilder, int> newLayer;
	for(const auto& mb : last) {
		// work on a motif
		++stNumMotifExplored;
		double score = scoring(mb, holder.lastScore()).second;
		if(score == numeric_limits<double>::lowest()) {
			// abandon if not promissing
			continue;
		} else if(holder.updatable(score)) {
			holder.update(mb.toMotif(), score);
			//if(flagDCESBound)
			//	maintainDCESBound(edges, holder.lastScore());
		}
		// generate new motifs if not abandon
		for(size_t i = 0; i < edges.size(); ++i) {
			const Edge& e = edges[i].first;
			if((mb.containNode(e.s) || mb.containNode(e.d)) && !mb.containEdge(e.s, e.d)) {
				MotifBuilder t(mb);
				t.addEdge(e.s, e.d);
				++newLayer[t];
				usedEdge[i] = true;
				++stNumMotifGenerated;
			}
		}
	}
	return newLayer;
}

std::pair<double, double> StrategyOFG::scoring(const MotifBuilder & mb, const double lowerBound)
{
	// TODO: optimize with parent selection and marked SD checking
	int cntPos = pdp->count(mb);
	double freqPos = static_cast<double>(cntPos) / pdp->size();
	double scoreUB = freqPos;
	// freqPos is the upperbound of differential & ratio based objective function
	//if(freqPos < minSup || scoreUB <= lowerBound)
	if(scoreUB <= lowerBound)
		return make_pair(numeric_limits<double>::lowest(), numeric_limits<double>::lowest());
	// calculate the how score
	int cntNeg = pdn->count(mb);
	double freqNeg = static_cast<double>(cntNeg) / pdn->size();
	return make_pair(scoreUB, objFun(freqPos, freqNeg));
}

std::pair<std::vector<MotifBuilder>, size_t> StrategyOFG::sortUpNewLayer(std::map<MotifBuilder, int>& layer)
{
	if(flagNetworkPrune)
		return pruneWithNumberOfParents(layer);
	else
		return removeDuplicate(layer);
}

std::pair<std::vector<MotifBuilder>, size_t> StrategyOFG::removeDuplicate(std::map<MotifBuilder, int>& layer)
{
	vector<MotifBuilder> res;
	res.reserve(layer.size());
	size_t cnt = 0;
	for(auto& p : layer) {
		res.push_back(move(p.first));
		cnt += p.second;
	}
	return make_pair(move(res), move(cnt));
}

int StrategyOFG::quickEstimiateNumberOfParents(const Motif & m)
{
/*	unordered_set<int> uniqueNodes, duplicateNodes;
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
	return uniqueNodes.size();*/
	unordered_map<int, int> cont;
	for(auto&e : m.edges) {
		++cont[e.s];
		++cont[e.d];
	}
	return count_if(cont.begin(), cont.end(), [](const pair<const int, int>& p) {return p.second == 1; });

}

int StrategyOFG::quickEstimiateNumberOfParents(const MotifBuilder & m)
{
	int cnt = 0;
	for(auto& p : m.nodes) {
		if(p.second == 1)
			++cnt;
	}
	return cnt;
}

/*
std::pair<std::vector<MotifBuilder>, size_t> StrategyOFG::pruneWithNumberOfParents(std::vector<MotifBuilder>& mbs)
{
	if(mbs.empty())
		return make_pair(mbs, 0);
	size_t cnt = mbs.size();
	sort(mbs.begin(), mbs.end());
	auto p = mbs.begin(); // point to the processing position
	auto pGroup = p; // point to the first of current group
	auto pRes = p; // point to an undetermined position
	auto pEnd = mbs.end();
	while(++p != pEnd) {
		if(!(*pGroup == *p)) { // new motif
			int nParentMin = quickEstimiateNumberOfParents(*pGroup);
			if(nParentMin <= p - pGroup) {
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
	return make_pair(move(mbs), move(cnt));
}
*/

std::pair<std::vector<MotifBuilder>, size_t> StrategyOFG::pruneWithNumberOfParents(std::map<MotifBuilder, int>& mbs)
{
	vector<MotifBuilder> res;
	size_t cnt = 0;
	for(auto& p : mbs) {
		int nParentMin = quickEstimiateNumberOfParents(p.first);
		int nGenerate = p.second;
		cnt += nGenerate;
		if(nParentMin <= nGenerate)
			res.push_back(move(p.first));
	}
	return make_pair(move(res), move(cnt));
}
