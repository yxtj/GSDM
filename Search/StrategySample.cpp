#include "stdafx.h"
#include "StrategySample.h"
#include "CandidateMethodFactory.h"
#include "CandidateMthdFreq.h"

using namespace std;

StrategySample::StrategySample()
{
}

std::vector<std::tuple<Motif, double, double>> StrategySample::search(
	const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg,
	const int smin, const int smax, const std::string& searchMethodyName, const CandidateMethodParm& par,
	const int k, const double pRefine)
{
	if(gPos.size() == 0 || gPos.front().size() == 0
		|| gNeg.size() == 0 || gNeg.front().size() == 0)
		return std::vector<std::tuple<Motif, double, double>>();

	cout << "Phase 1 (sameple some sparse graphs):" << endl;
	vector<int> sampled = sampleGraphs(gPos, 0.5);
	sort(sampled.begin(), sampled.end());
	cout << sampled.size() << " graphs are sampled" << endl;

	cout << "Phase 2 (find positive on sampled):" << endl;
	CandidateMethod* method = CandidateMethodFactory::generate(searchMethodyName);
	vector<vector<pair<Motif, double> > > phase2;
	for(size_t i = 0; i < sampled.size(); ++i) {
		chrono::system_clock::time_point _time = chrono::system_clock::now();
		phase2.push_back(candidateFromOne(gPos[sampled[i]], smin, smax, method, par));
		auto _time_ms = chrono::duration_cast<chrono::milliseconds>(
			chrono::system_clock::now() - _time).count();
		cout << "  On individual " << sampled[i] << " found " << phase2[i].size()
			<< " motifs within " << _time_ms << " ms" << endl;
	}
	delete method;

	cout << "Phase 3 (refine frequent):" << endl;
	vector<tuple<Motif, double, double>> phase3 = refineByAll(phase2, pRefine / 2);
	phase2.clear();
	cout << phase3.size() << " motifs after refinement." << endl;

	cout << "Phase 3' (refine on all positive):" << endl;
	double pMin = static_cast<const CandidateMthdFreqParm&>(par).pMin;
//	phase3 = checkMotifOnAll(phase3, gPos, sampled, pMin, pRefine);
	cout << phase3.size() << " motifs after refinement." << endl;

	ofstream fout("graph.txt");
	for(auto& tp : phase3) {
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


	cout << "Phase 4 (filter by negative infrequent):" << endl;
	double pMax = 0.5;
	vector<tuple<Motif, double, double>> phase4 = filterByNegative(phase3, pMax, gNeg);
	phase3.clear();
	cout << phase4.size() << " motifs after removal of negative frequent ones." << endl;

	return phase4;
}

std::vector<int> StrategySample::sampleGraphs(const std::vector<std::vector<Graph>>& gs, const double por)
{
	vector<double> density(gs.size());
	vector<int> offset(gs.size());
	size_t nRes = static_cast<size_t>(por*gs.size());
	int nNode = gs.front().front().nEdge;
	for(size_t i = 0; i < gs.size(); ++i) {
		int nEdge = 0;
		for(const Graph& g : gs[i])
			nEdge += g.nEdge;
		density[i]=static_cast<double>(nEdge) / gs[i].size();
		offset[i] = i;
	}
	// get the sparsest nRes graphs
	sort(offset.begin(), offset.end(), [&density](const int r, const int l) {
		return density[r] < density[l];
	});
	offset.resize(nRes);
	return offset;
}

std::vector<std::pair<Motif, double>> StrategySample::candidateFromOne(
	const std::vector<Graph>& gs, int smin, int smax,
	CandidateMethod * method, const CandidateMethodParm & par)
{
	return method->getCandidantMotifs(gs, smin, smax, par);
}

std::vector<std::tuple<Motif, double, double>> StrategySample::refineByAll(
	std::vector<std::vector<std::pair<Motif, double>>>& motifs, const double pRef)
{
	// count occurrence of each motif
	map<Motif, pair<int, double>> cont;
	for(auto it = motifs.begin(); it != motifs.end(); ++it) {
		for(auto jt = it->begin(); jt != it->end(); ++jt) {
			cont[jt->first].first++;
			cont[jt->first].second += jt->second;
		}
		it->clear();
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

std::vector<std::tuple<Motif, double, double>> StrategySample::checkMotifOnAll(
	std::vector<std::tuple<Motif, double, double>>& motifs,
	const std::vector<std::vector<Graph>> & gs, const std::vector<int>& sampled,
	const double pMin, const double pRef)
{
	vector<tuple<Motif, double, double>> res;
	for(std::tuple<Motif, double, double>& t : motifs) {
		size_t p = 0;
		int cnt = 0;
		double ap = 0.0;
		for(size_t i = 0; i < gs.size(); ++i) {
			//if(p<sampled.size() && i >= sampled[p]) {
			//	++p;
			//	continue;
			//}
			double px = CandidateMethod::probOfMotif(get<0>(t), gs[i]);
			ap += px;
			if(px > pMin)
				cnt++;
		}
		//double pAvg = (ap + get<2>(t) * sampled.size()) / gs.size();
		double pAvg = ap / gs.size();
		get<2>(t) = pAvg;
		//double pMotif = (cnt + get<1>(t) * sampled.size()) / gs.size();
		double pMotif = static_cast<double>(cnt) / gs.size();
		get<1>(t) = pMotif;
		if(pMotif > pRef) {
			res.push_back(move(t));
		}
	}
	return res;
}

std::vector<std::tuple<Motif, double, double>> StrategySample::filterByNegative(
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


