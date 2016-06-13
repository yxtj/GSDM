#include "stdafx.h"
#include "SearcherFreqPN.h"
#include "CandidateMethodFactory.h"
#include "CandidateFreq.h"
#include "StrategyInfreq.h"

using namespace std;

SearcherFreqPN::SearcherFreqPN()
{
}

std::vector<std::tuple<Motif, double, double>> SearcherFreqPN::search(
	const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg,
	const int smin, const int smax, const std::string& searchStrategyName, const SearchStrategyPara& par,
	const int k, const double pRefine)
{
	if(gPos.size() == 0 || gPos.front().size() == 0
		|| gNeg.size() == 0 || gNeg.front().size() == 0)
		return std::vector<tuple<Motif, double, double>>();
	// initial probability graphs for future usage

	// searching
	CandidateMethod* strategy = CandidateMethodFactory::generate(searchStrategyName);
	vector<vector<pair<Motif, double> > > phase1;
	cout << "Phase 1 (find positive):" << endl;
	for(size_t i = 0; i < gPos.size(); ++i) {
		phase1.push_back(candidateFromOne(gPos[i], smin, smax, strategy, par));
		cout << "  On individual " << i << " found " << phase1[i].size() << " motifs." << endl;
	}
	delete strategy;

	cout << "Phase 2 (refine frequent):" << endl;
	vector<tuple<Motif, double, double>> phase2 = refineByAll(phase1, k, pRefine);
	cout << phase2.size() << " motifs after refinement." << endl;

	cout << "Phase 3 (filter by negative infrequent):" << endl;
	double pdis = 0.8; // (x/(x+y)>pdis
	double pMax = static_cast<const StrategyFreqPara&>(par).pMin * (1.0 / pdis - 1);
	vector<tuple<Motif, double, double>> phase3 = filterByNegative(phase2, pMax, gNeg);
	cout << phase3.size() << " motifs after removal of negative frequent ones." << endl;

	return phase3;

	// search negative
/*
// 	double pDis = 0.8;
// 	double pX = static_cast<const StrategyInfreqPara&>(par).pMin;
// 	StrategyInfreqPara* parNeg = new StrategyInfreqPara();
// 	parNeg->pMin = pX / pDis - pX;
// 
// 	SearchStrategy* strategyN = SearchStrategyFactory::generate("Infreq");
// 	vector<vector<pair<Motif, double> > > phase3;
// 	cout << "Phase 3 (find negative):" << endl;
// 	for(size_t i = 0; i < gNeg.size(); ++i) {
// 		phase3.push_back(candidateFromOne(gNeg[i], smin, smax, strategyN, *parNeg));
// 		cout << "  On individual " << i << " found " << phase3[i].size() << " motifs." << endl;
// 	}
// 	delete strategyN;
// 	delete parNeg;
// 
// 	cout << "Phase 4 (refine infrequent):" << endl;
// 	vector<tuple<Motif, double, double>> phase4 = refineByAll(phase3, k, pRefine);
// 
// 	// remove negative from positive
// 	vector<tuple<Motif, double, double>> phase5;
// 	for(const auto& tpos : phase2) {
// 		bool useful = true;
// 		for(const auto& tneg : phase4) {
// 			if(get<0>(tpos) == get<0>(tneg)) {
// 				useful = false;
// 				break;
// 			}
// 		}
// 		if(useful)
// 			phase5.push_back(tpos);
// 	}

	return phase5;
*/

}


std::vector<std::pair<Motif, double>> SearcherFreqPN::candidateFromOne(const std::vector<Graph> & gs,
	int smin, int smax, CandidateMethod* strategy, const SearchStrategyPara& par)
{
	return strategy->getCandidantMotifs(gs, smin, smax, par);
}

std::vector<std::tuple<Motif, double, double>> SearcherFreqPN::filterByNegative(
	const std::vector<std::tuple<Motif, double, double>>& motifs, 
	const double pMax, const std::vector<std::vector<Graph>>& gNeg)
{
	std::vector<std::tuple<Motif, double, double>> res;
	for(auto &tp : motifs) {
		double d = 0.0;
		for(auto& line : gNeg) {
			d += CandidateMethod::probOfMotif(get<0>(tp), line);
		}
		d /= gNeg.size();
		if(d <= pMax)
			res.push_back(move(tp));
	}
	return res;
}

std::vector<std::tuple<Motif, double, double>> SearcherFreqPN::refineByAll(
	const std::vector<std::vector<std::pair<Motif, double>>>& motifs, const int k, const double pRef)
{
	// count occurrence of each motif
	map<Motif, pair<int, double>> cont;
	for(auto it = motifs.begin(); it != motifs.end(); ++it) {
		for(auto jt = it->begin(); jt != it->end(); ++jt) {
			cont[jt->first].first++;
			cont[jt->first].second += jt->second;
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
