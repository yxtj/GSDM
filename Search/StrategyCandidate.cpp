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
	chrono::system_clock::time_point _time = chrono::system_clock::now();
	vector<tuple<Motif, double, double>> phase2 = refineByAll(phase1);

	vector<Motif> res;
	for(auto& tp : phase2) {
		res.push_back(move(get<0>(tp)));
	}
	auto _time_ms = chrono::duration_cast<chrono::milliseconds>(
		chrono::system_clock::now() - _time).count();
	cout << "  refine and pick top " << res.size()
		<< " motifs within " << _time_ms << " ms" << endl;
	return res;

*/
	cout << "Phase 1 (find all)" << endl;
	unordered_map<Motif, pair<int, double>> phase1 = freqOnSet(method, gPos);
	delete method;

	cout << "Phase 2 (refine & pick top k)" << endl;
	chrono::system_clock::time_point _time = chrono::system_clock::now();
	vector<Motif> phase2 = pickTopK(phase1, gPos.size());
	auto _time_ms = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - _time).count();
	cout << "  pick top " << phase2.size() << " motifs within " << _time_ms << " ms" << endl;
	return phase2;

}

std::vector<std::pair<Motif, double>> StrategyCandidate::candidateFromOne(
	CandidateMethod* method, const std::vector<Graph> & gs)
{
	return method->getCandidantMotifs(gs);
}

std::unordered_map<Motif, std::pair<int, double>> StrategyCandidate::freqOnSet(
	CandidateMethod* method, const std::vector<std::vector<Graph>>& gs)
{
	unordered_map<Motif, pair<int, double>> phase1;
	for(size_t i = 0; i < gs.size(); ++i) {
		chrono::system_clock::time_point _time = chrono::system_clock::now();
		auto vec = method->getCandidantMotifs(gs[i]);
		countMotif(phase1, vec);
		auto _time_ms = chrono::duration_cast<chrono::milliseconds>(
			chrono::system_clock::now() - _time).count();
		cout << "  On individual " << i << " found " << vec.size()
			<< " motifs within " << _time_ms << " ms"
			<< ". All unique motifs: " << phase1.size() << endl;
	}
	return phase1;
}

void StrategyCandidate::countMotif(
	std::unordered_map<Motif, std::pair<int, double>>& res, std::vector<std::pair<Motif, double>>& vec)
{
	for(auto& p : vec) {
		auto& ref = res[p.first];
		ref.first++;
		ref.second += p.second;
	}
}

std::vector<Motif> StrategyCandidate::pickTopK(
	std::unordered_map<Motif, std::pair<int, double>>& data, const size_t gsize)
{
	vector<pair<int, decltype(data.begin())>> idx;
	int minOcc = static_cast<int>(ceil(pRefine*gsize));
	auto it = data.begin();
	for(size_t i = 0; i < data.size(); ++i, ++it) {
		if(it->second.first >= minOcc) {
			it->second.second /= it->second.first;
			idx.emplace_back(i, it);
		}
	}
	sort(idx.begin(), idx.end(),
		[](const pair<int, decltype(data.begin())>& a, const pair<int, decltype(data.begin())>& b) {
		return a.first > b.first || a.first == b.first && a.second->second > b.second->second;
		//return a.first > b.first;
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
//	map<Motif, pair<int,double>> contGen;
	unordered_map<Motif, pair<int, double>> contGen;
	for(auto it = motifs.begin(); it != motifs.end(); ++it) {
		for(auto jt = it->begin(); jt != it->end(); ++jt) {
			contGen[jt->first].first++;
			contGen[jt->first].second+=jt->second;
		}
	}
	// sort motifs by occurrence count (remove those occurred too infrequent)
	const int minFre = static_cast<int>(pRefine*motifs.size());
	double dev = motifs.size();
	vector<tuple<Motif, double, double>> res;
	for(auto it = contGen.begin(); it != contGen.end(); ++it) {
		if(it->second.first > minFre) {
			it->second.second /= it->second.first;
			res.emplace_back(move(it->first), it->second.first/dev, it->second.second);
		}
	}
	contGen.clear();
	sort(res.begin(), res.end(),
		[](const tuple<Motif, double, double>& a, const tuple<Motif, double, double>& b) {
		return get<1>(a) > get<1>(b) || get<1>(a) == get<1>(b) && get<2>(a) == get<2>(b);
	});
	return res;
}

