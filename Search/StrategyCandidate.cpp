#include "stdafx.h"
#include "StrategyCandidate.h"
#include "CandidateMethodFactory.h"
#include "Option.h"

using namespace std;

const std::string StrategyCandidate::name("candidate");
const std::string StrategyCandidate::usage(
	"Select the common frequent motifs as result.\n"
	"Usage: " + StrategyCandidate::name + " <# of result> <occurence ratio>\n"
	"  <OC>: used to refine the motifs among subjects");

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
/*
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
*/
	map<Motif, pair<int, double>> phase1;
	cout << "Phase 1 (find all)" << endl;
	for(size_t i = 0; i < gPos.size(); ++i) {
		chrono::system_clock::time_point _time = chrono::system_clock::now();
		auto vec = method->getCandidantMotifs(gPos[i]);
		countMotif(phase1, vec);
		auto _time_ms = chrono::duration_cast<chrono::milliseconds>(
			chrono::system_clock::now() - _time).count();
		cout << "  On individual " << i << " found " << vec.size()
			<< " motifs within " << _time_ms << " ms"
			<< ". All unique motifs: " << phase1.size() << endl;
	}
	delete method;

	cout << "Phase 2 (pick top k)" << endl;
	vector<Motif> phase2 = pickTopK(phase1, gPos.size());
	return phase2;
}

std::vector<std::pair<Motif, double>> StrategyCandidate::candidateFromOne(
	CandidateMethod* method, const std::vector<Graph> & gs)
{
	return method->getCandidantMotifs(gs);
}

void StrategyCandidate::countMotif(std::map<Motif, std::pair<int, double>>& res, std::vector<std::pair<Motif, double>>& vec)
{
	for(auto& p : vec) {
		auto& ref = res[p.first];
		ref.first++;
		ref.second += p.second;
	}
}

std::vector<Motif> StrategyCandidate::pickTopK(
	std::map<Motif, std::pair<int, double>>& data, const size_t gsize)
{
	vector<pair<int, decltype(data.begin())>> idx;
	int minOcc = static_cast<int>(ceil(pRefine*gsize));
	auto it = data.begin();
	for(size_t i = 0; i < data.size(); ++i, ++it) {
		if(it->second.first >= minOcc) {
			//it->second.second /= it->second.first;
			idx.emplace_back(i, it);
		}
	}
	sort(idx.begin(), idx.end(),
		[](const pair<int, decltype(data.begin())>& a, const pair<int, decltype(data.begin())>& b) {
		return a.first > b.first;
		//return a.first > b.first || a.first == b.first && a.second->second > b.second->second;
	});
	cout << "  valid motif: " << idx.size() << endl;

	vector<Motif> res;
	size_t end = min(static_cast<size_t>(k), idx.size());
	for(size_t i = 0; i < end; ++i)
		res.push_back(move(idx[i].second->first));
	return res;
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

