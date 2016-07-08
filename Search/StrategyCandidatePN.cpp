#include "stdafx.h"
#include "StrategyCandidatePN.h"
#include "CandidateMethodFactory.h"
#include "CandidateMthdFreq.h"
#include "Option.h"

using namespace std;

const std::string StrategyCandidatePN::name("candidatePN");
const std::string StrategyCandidatePN::usage(
	"Select the common frequent motifs as result.\n"
	"Usage: " + StrategyCandidatePN::name + " <# of result> <ratio Pos.> <acpt Neg.> <ratio Neg.>\n"
	"  <RP>: minium motif occurence among positive samples\n"
	"  <AP>: minium show-up ratio among negative snapshots, used to judge occurence or not\n"
	"  <RN>: maxium motif occurence among negative samples\n");

bool StrategyCandidatePN::parse(const std::vector<std::string>& param)
{
	try {
		checkParam(param, 4, name);
		k = stoi(param[1]);
		pRefine = stod(param[2]);
		pPickNeg = stod(param[3]);
		pRefineNeg = stod(param[4]);
	} catch(exception& e) {
		cerr << e.what() << endl;
		return false;
	}
	return true;
}

std::vector<Motif> StrategyCandidatePN::search(const Option& opt,
	const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg)
{
	if(!checkInput(gPos, gNeg))
		return std::vector<Motif>();
	// initial probability graphs for future usage

	// searching
	CandidateMethod* method = CandidateMethodFactory::generate(opt.getMethodName());
	method->parse(opt.mtdParam);

	cout << "Phase 1 (find positive):" << endl;
	unordered_map<Motif, pair<int, double>> phase1 = freqOnSet(method, gPos);
	delete method;

	cout << "Phase 2 (refine frequent):" << endl;
	//vector<tuple<Motif, double, double>> phase2 = refineByAll(phase1);
	vector<Motif> phase2 = pickTopK(phase1, gPos.size());
	cout << phase2.size() << " motifs after refinement." << endl;

/*	ofstream fout(opt.prefix + "graph.txt");
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
	fout.close();*/

	cout << "Phase 3 (filter out negative frequent ones):" << endl;
	vector<Motif> phase3 = filterByNegative(phase2, gNeg);
	phase2.clear();
	cout << phase3.size() << " motifs after removal of negative frequent ones." << endl;

	cout << "Phase 4 (pick top k):" << endl;
	vector<Motif> phase4;
	if(phase3.size() <= static_cast<size_t>(k)) {
		phase4 = move(phase3);
	} else {
		for(int i = 0; i < k; ++i) {
			phase4.push_back(move(phase3[i]));
		}
	}

	return phase4;
}

std::vector<std::pair<Motif, double>> StrategyCandidatePN::candidateFromOne(
	CandidateMethod* method, const std::vector<Graph> & gs)
{
	return method->getCandidantMotifs(gs);
}

std::unordered_map<Motif, std::pair<int, double>> StrategyCandidatePN::freqOnSet(
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

void StrategyCandidatePN::countMotif(
	std::unordered_map<Motif, std::pair<int, double>>& res, std::vector<std::pair<Motif, double>>& vec)
{
	for(auto& p : vec) {
		auto& ref = res[p.first];
		ref.first++;
		ref.second += p.second;
	}
}

std::vector<Motif> StrategyCandidatePN::pickTopK(
	std::unordered_map<Motif, std::pair<int, double>>& data, const size_t gsize, const int k)
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

	vector<Motif> res;
	size_t end = min(static_cast<size_t>(k), idx.size());
	for(size_t i = 0; i < end; ++i)
		res.push_back(move(idx[i].second->first));
	return res;
}


std::vector<Motif> StrategyCandidatePN::filterByNegative(
	std::vector<Motif>& motifs,
	const std::vector<std::vector<Graph>>& gNeg)
{
	std::vector<Motif> res;
	const int acpt = static_cast<int>(ceil(pRefineNeg*gNeg.size()));
	for(auto &m : motifs) {
		int cnt = 0;
		for(auto& line : gNeg) {
			double d = CandidateMethod::probOfMotif(m, line);
			if(d >= pPickNeg)
				++cnt;
		}
		if(cnt >= acpt)
			res.push_back(move(m));
	}
	return res;
}
