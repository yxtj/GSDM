#include "stdafx.h"
#include "StrategyCandidate.h"
#include "CandidateMethodFactory.h"
#include "Option.h"

using namespace std;

const std::string StrategyCandidate::name("candidate");
const std::string StrategyCandidate::usage(
	"Select the common frequent motifs as result.\n"
	"  " + StrategyCandidate::name + " <# of result> <occurence ratio>\n"
	"<OC>: used to refine the motifs among subjects");

bool StrategyCandidate::parse(const std::vector<std::string>& param)
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

std::vector<Motif> StrategyCandidate::search(const Option& opt, 
	const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg)
{
	if(!checkInput(gPos, gNeg))
		return std::vector<Motif>();
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

	CandidateMethod* method = CandidateMethodFactory::generate(opt.getMethodName());
	method->parse(opt.mtdParam);

	vector<vector<pair<Motif, double> > > phase1;
	cout << "Phase 1 (find positive):" << endl;
	for(size_t i = 0; i < gPos.size(); ++i) {
		chrono::system_clock::time_point _time = chrono::system_clock::now();
		phase1.push_back(method->getCandidantMotifs(gPos[i]));
//			candidateFromOne(gPos[i], smin, smax, method, par));
		auto _time_ms = chrono::duration_cast<chrono::milliseconds>(
			chrono::system_clock::now() - _time).count();
		cout << "  On individual " << i << " found " << phase1[i].size()
			<< " motifs within " << _time_ms << " ms" << endl;
	}
	delete method;

	cout << "Phase 2 (refine frequent):" << endl;
	vector<tuple<Motif, double, double>> phase2 = refineByAll(phase1);

	vector<Motif> res;
	for(auto& tp : phase2) {
		res.push_back(move(get<0>(tp)));
	}
	return res;

}

std::vector<std::pair<Motif, double>> StrategyCandidate::candidateFromOne(
	CandidateMethod* method, const std::vector<Graph> & gs)
{
	return method->getCandidantMotifs(gs);
}

std::vector<std::tuple<Motif, double, double>> StrategyCandidate::refineByAll(
	const std::vector<std::vector<std::pair<Motif,double>>>& motifs)
{
	// count occurrence of each motif
	map<Motif, pair<int,double>> contGen;
	for(auto it = motifs.begin(); it != motifs.end(); ++it) {
		for(auto jt = it->begin(); jt != it->end(); ++jt) {
			contGen[jt->first].first++;
			contGen[jt->first].second+=jt->second;
		}
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

