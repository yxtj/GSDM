#include "stdafx.h"
#include "StrategyOFGPara.h"
#include "MType.h"

using namespace std;

void StrategyOFGPara::initialCE_para()
{
	int size = net->size();
	int id = net->id();
	// split the edge space evenly and check them individually

	int nEdgeInAll = nNode*(nNode - 1) / 2;
	pair<int, int> cef = num2Edge(static_cast<int>(floor(nEdgeInAll / (double)size)*id));
	pair<int, int> cel = num2Edge(static_cast<int>(floor(nEdgeInAll / (double)size)*(id + 1)));

	double factor = 1.0 / pgp->size();
	int th = static_cast<int>(ceil(minSup*pgp->size()));
	th = max(th, 1); // in case of minSup=0
	vector<tuple<Edge, double, int>> ceLocal;
	for(int i = cef.first; i <= cel.first; ++i) {
		int j = (i == cef.first ? cef.second : i + 1);
		int endj = (i == cel.first ? cel.second : nNode);
		while(j < endj) {
			++st.nEdgeChecked;
			++st.nFreqPos;
			int t = countEdgeXSub(i, j, *pgp);
			if(t >= th) {
				auto f = t*factor;
				ceLocal.emplace_back(Edge(i, j), f, 0);
			}
			++j;
		}
	}

	// shared the candidate edges among all workers
	rph.input(MType::CEInit, net->id());
	net->broadcast(MType::CEInit, ceLocal);
	{
		lock_guard<mutex> lg(mce);
		if(edges.empty())
			edges = ceLocal;
		else
			move(ceLocal.begin(), ceLocal.end(), back_inserter(edges));
	}
	suCEinit.wait();
	// sort edge with decreasing order
	sort(edges.begin(), edges.end(),
		[](const tuple<Edge, double, int>& lth, const tuple<Edge, double, int>& rth) {
		return get<1>(lth) > get<1>(rth);
	});
	st.progCESize.emplace_back(timer.elapseMS(), edges.size());
	// check trivial case
	if(edges.empty()) {
		throw invalid_argument("No candidate edge available. Directly exit.");
	}
}

std::pair<int, int> StrategyOFGPara::num2Edge(const int idx)
{
	// i, j and idx all start from 0
	if(idx == nNode*(nNode - 1) / 2)
		// the normal look cannot finish for this case
		return make_pair(nNode - 2, nNode - 1);
	int i = 0;
	int n = nNode - i - 1;
	while(idx >= n) {
		++i;
		n += nNode - i - 1;
	}
	int j = idx - n + nNode;
	return make_pair(i, j);
}

void StrategyOFGPara::edgeUsageSend(const int since)
{
	vector<pair<Edge, int>> update;
	update.reserve(edges.size());
	{
		lock_guard<mutex> lg(mce);
		for(auto& et : edges) {
			// mark > since
			if(get<2>(et) >= since) {
				update.emplace_back(get<0>(et), get<2>(et));
			}
		}
	}
	net->broadcast(MType::CEUsage, update);
}

void StrategyOFGPara::edgeUsageUpdate(const std::vector<std::pair<Edge, int>>& usage)
{
	auto fu = usage.begin(), lu = usage.end();
	lock_guard<mutex> lg(mce);
	auto fe = edges.begin(), le = edges.end();
	// rationale: edges and usage are sorted with 
	while(fe != le && fu != lu) {
		if(fu->first < get<0>(*fe)) {
			++fu;
		} else if(get<0>(*fe) < fu->first) {
			++fe;
		} else {
			get<2>(*fe) = max(get<2>(*fe), fu->second);
			++fe;
			++fu;
		}
	}
}

void StrategyOFGPara::removeUnusedEdges()
{
	lock_guard<mutex> lg(mce);
	auto it = remove_if(edges.begin(), edges.end(),
		[=](const tuple<Edge, double, int>& t) {
		return get<2>(t) < *lastFinishLevel;
	});
	if(it != edges.end()) {
		edges.erase(it, edges.end());
		st.progCESize.emplace_back(timer.elapseMS(), edges.size());
	}
}

void StrategyOFGPara::removeGivenEdges(const std::vector<Edge>& given)
{
	lock_guard<mutex> lg(mce);
	auto it = remove_if(edges.begin(), edges.end(),
		[&](const tuple <Edge, double, int>& p) {
		return find(given.begin(), given.end(), get<0>(p)) != given.end();
	});
	if(it != edges.end()) {
		edges.erase(it, edges.end());
		st.progCESize.emplace_back(timer.elapseMS(), edges.size());
	}
}
