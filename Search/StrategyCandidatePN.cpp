#include "stdafx.h"
#include "StrategyCandidatePN.h"
#include "CandidateMethodFactory.h"
#include "CandidateMthdFreq.h"
#include "Option.h"

using namespace std;

const std::string StrategyCandidatePN::name("candidatePN");
const std::string StrategyCandidatePN::usage(
	"Select the common frequent motifs as result.\n"
	"Usage: " + StrategyCandidatePN::name + " <# of result> <occurence ratio>\n"
	"  <OC>: used to refine the motifs among subjects");

bool StrategyCandidatePN::parse(const std::vector<std::string>& param)
{
	try {
		checkParam(param, 2, name);
		k = stoi(param[1]);
		pRefine = stod(param[2]);
	} catch(exception& e) {
		cerr << e.what() << endl;
		return false;
	}
	return true;
}

std::vector<Motif> StrategyCandidatePN::search(const Option& opt,
	const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg)
{
	if(checkInput(gPos, gNeg))
		return std::vector<Motif>();
	// initial probability graphs for future usage

	// searching
	CandidateMethod* method = CandidateMethodFactory::generate(opt.getMethodName());

	vector<vector<pair<Motif, double> > > phase1;
	cout << "Phase 1 (find positive):" << endl;
	for(size_t i = 0; i < gPos.size(); ++i) {
		chrono::system_clock::time_point _time = chrono::system_clock::now();
		phase1.push_back(candidateFromOne(method, gPos[i]));
		auto _time_ms = chrono::duration_cast<chrono::milliseconds>(
			chrono::system_clock::now() - _time).count();
		cout << "  On individual " << i << " found " << phase1[i].size()
			<< " motifs within " << _time_ms << " ms" << endl;
	}
	delete method;

	cout << "Phase 2 (refine frequent):" << endl;
	vector<tuple<Motif, double, double>> phase2 = refineByAll(phase1);
	cout << phase2.size() << " motifs after refinement." << endl;

	ofstream fout(opt.prefix + "graph.txt");
	for(auto& tp : phase2) {
		Motif& m = get<0>(tp);
		fout << m.getnNode() << "\t" << m.getnEdge() << "\t"
			<< std::fixed << get<1>(tp) << "\t"
			<< std::fixed << get<2>(tp) << "\t";
		for(const Edge&e : m.edges) {
			fout << "(" << e.s << "," << e.d << ") ";
		}
		fout << "\n";
	}
	fout.close();

	cout << "Phase 3 (filter by negative infrequent):" << endl;
	double disScore = 0.5; // (x/(x+y)>disScore
	//double pMax = static_cast<const CandidateMthdFreqParm&>(par).pMin * (1.0 / disScore - 1);
	double pMax = 0.5;
	vector<tuple<Motif, double, double>> phase3 = filterByNegative(phase2, pMax, gNeg);
	cout << phase3.size() << " motifs after removal of negative frequent ones." << endl;

	vector<Motif> res;
	for(auto& tp : phase3) {
		res.push_back(move(get<0>(tp)));
	}

	return res;

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


std::vector<std::pair<Motif, double>> StrategyCandidatePN::candidateFromOne(
	CandidateMethod* method, const std::vector<Graph> & gs)
{
	return method->getCandidantMotifs(gs);
}

std::vector<std::tuple<Motif, double, double>> StrategyCandidatePN::refineByAll(
	std::vector<std::vector<std::pair<Motif, double>>>& motifs)
{
	// count occurrence of each motif
	map<Motif, pair<int, double>> contGen;
	for(auto it = motifs.begin(); it != motifs.end(); ++it) {
		for(auto jt = it->begin(); jt != it->end(); ++jt) {
			contGen[jt->first].first++;
			contGen[jt->first].second += jt->second;
		}
		it->clear();
	}
	// sort motifs by occurrence count (remove those occurred too infrequent)
	const int minFre = static_cast<int>(pRefine*motifs.size());
	multimap<int, decltype(contGen.begin())> valid;
	for(auto it = contGen.begin(); it != contGen.end(); ++it) {
		if(it->second.first > minFre) {
			valid.emplace(it->second.first, it);
		}
	}
	// generate output
	double dev = motifs.size();
	vector<tuple<Motif, double, double>> res;
	res.reserve(valid.size());
	int count = 0;
	for(auto it = valid.rbegin(); it != valid.rend(); ++it) {
		res.emplace_back(move(it->second->first), it->first / dev, it->second->second.second / dev);
	}
	// sort with <pIndividual, meanPHappen>
	sort(res.begin(), res.end(), 
		[](const tuple<Motif, double, double>& l, const tuple<Motif, double, double>& r) {
			return get<1>(l) > get<1>(r) ? true : get<1>(l) == get<1>(r) && get<2>(l) > get<2>(r);
	});
	return res;
}

std::vector<std::tuple<Motif, double, double>> StrategyCandidatePN::filterByNegative(
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
