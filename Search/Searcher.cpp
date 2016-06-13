#include "stdafx.h"
#include "Searcher.h"
#include "CandidateMethodFactory.h"

using namespace std;

Searcher::Searcher()
{
}

std::vector<std::tuple<Motif,double,double>> Searcher::search(
	const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg,
	const int smin, const int smax, const std::string& searchStrategyName, const SearchStrategyPara& par,
	const int k, const double pRefine)
{
	if(gPos.size()==0 || gPos.front().size() == 0 
		|| gNeg.size()==0 || gNeg.front().size() == 0)
		return std::vector<tuple<Motif, double, double>>();
	// initial probability graphs for future usage
	/*
	vector<GraphProb> gpp, gpn;
	GraphProb gppt, gpnt; // gp on all graphs
	for(const auto& line : gPos) {
		GraphProb t(line.front().nNode);
		for(const Graph& g : line) {
			t.iterAccum(g);
			gppt.iterAccum(g);
		}
		t.finishAccum();
		gpp.push_back(move(t));
	}
	gppt.finishAccum();
	for(const auto& line : gNeg) {
		GraphProb t(line.front().nNode);
		for(const Graph& g : line) {
			t.iterAccum(g);
			gpnt.iterAccum(g);
		}
		t.finishAccum();
		gpn.push_back(move(t));
	}
	gpnt.finishAccum();
	*/

	// searching
	CandidateMethod* strategy = CandidateMethodFactory::generate(searchStrategyName);
	vector<vector<pair<Motif,double> > > phase1;
	cout << "Phase 1 (find positive):"<<endl;
	for(size_t i = 0; i < gPos.size(); ++i) {
		phase1.push_back(candidateFromOne(gPos[i], smin, smax, strategy, par));
		cout << "  On individual " << i << " found " << phase1[i].size() << " motifs." << endl;
	}
	delete strategy;

	cout << "Phase 2 (refine frequent):" << endl;
	vector<tuple<Motif, double, double>> phase2 = refineByAll(phase1, k, pRefine);

	return phase2;
}


std::vector<std::pair<Motif, double>> Searcher::candidateFromOne(const std::vector<Graph> & gs,
	int smin, int smax, CandidateMethod* strategy, const SearchStrategyPara& par)
{
	return strategy->getCandidantMotifs(gs, smin, smax, par);
}

std::vector<std::tuple<Motif, double, double>> Searcher::refineByAll(
	const std::vector<std::vector<std::pair<Motif,double>>>& motifs, const int k, const double pRef)
{
	// count occurrence of each motif
	map<Motif, pair<int,double>> cont;
	for(auto it = motifs.begin(); it != motifs.end(); ++it) {
		for(auto jt = it->begin(); jt != it->end(); ++jt) {
			cont[jt->first].first++;
			cont[jt->first].second+=jt->second;
		}
	}
	// sort motifs by occurrence count (remove those occurred too infrequent)
	const int minFre = static_cast<int>(pRef*motifs.size());
	multimap<int, decltype(cont.begin())> valid;
	for(auto it = cont.begin(); it != cont.end(); ++it) {
		if(it->second.first > minFre) {
			valid.emplace(it->second.first, it);
		}
	}
	// generate output
	const int maxV = max<int>(k, valid.size());
	double dev = motifs.size();
	vector<tuple<Motif, double, double>> res;
	res.reserve(maxV);
	int count = 0;
	for(auto it = valid.rbegin(); it != valid.rend(); ++it) {
		res.emplace_back(move(it->second->first), it->first / dev, it->second->second.second / dev);
		if(++count >= k)
			break;
	}
	sort(res.begin(), res.end(), [](const tuple<Motif, double, double>& l, const tuple<Motif, double, double>& r) {
		return get<1>(l) > get<1>(r) ? true : get<1>(l) == get<1>(r) && get<2>(l) > get<2>(r);
	});
	return res;
}
