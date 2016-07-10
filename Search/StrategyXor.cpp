#include "stdafx.h"
#include "StrategyXor.h"
#include "CandidateMethodFactory.h"
#include "Option.h"
#include "MotifCommunicator.h"

const std::string StrategyXor::name("xor");
const std::string StrategyXor::usage(
	"get the symmetric difference of the frequent sets between positive samples and negative samples.\n"
	"Usage: " + StrategyXor::name + " <occurence ratio>\n"
	"  <OC>: used to refine the motifs among subjects");

using namespace std;

bool StrategyXor::parse(const std::vector<std::string>& param)
{
	try {
		checkParam(param, 1, name);
		pRefine = stod(param[1]);
	} catch(exception& e) {
		cerr << e.what() << endl;
		return false;
	}
	return true;
}

std::vector<Motif> StrategyXor::search(const Option& opt,
	const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg)
{
	if(!checkInput(gPos, gNeg))
		return std::vector<Motif>();
	CandidateMethod* method = CandidateMethodFactory::generate(opt.getMethodName());
	method->parse(opt.mtdParam);

	cout << "Phase 1 (find postive)" << endl;
	unordered_map<Motif, pair<int, double>> phase1 = freqOnSet(method, gPos);

	cout << "Phase 2 (refine postive)" << endl;
	chrono::system_clock::time_point _time = chrono::system_clock::now();
	vector<Motif> phase2 = refineByPostive(phase1, gPos.size());
	phase1.clear();
	auto _time_ms = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - _time).count();
	cout << "  refined " << phase2.size() << " motifs within " << _time_ms << " ms" << endl;

	cout << "Phase 3 (find negative)" << endl;
	unordered_map<Motif, pair<int, double>> phase3 = freqOnSet(method, gNeg);

	cout << "Phase 4 (refine negative)" << endl;
	_time = chrono::system_clock::now();
	vector<Motif> phase4 = refineByPostive(phase3, gPos.size());
	phase3.clear();
	_time_ms = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - _time).count();
	cout << "  refined " << phase4.size() << " motifs within " << _time_ms << " ms" << endl;

	cout << "Phase 5 (intersect)" << endl;
	_time = chrono::system_clock::now();
	vector<Motif> phase5 = symmetricDifference(phase2, phase4);
	_time_ms = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - _time).count();
	cout << "  intersected " << phase5.size() << " motifs within " << _time_ms << " ms" << endl;

	delete method;
	return phase5;

}

std::unordered_map<Motif, std::pair<int, double>> StrategyXor::freqOnSet(
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

void StrategyXor::countMotif(
	std::unordered_map<Motif, std::pair<int, double>>& res, std::vector<std::pair<Motif, double>>& vec)
{
	for(auto& p : vec) {
		auto& ref = res[p.first];
		ref.first++;
		ref.second += p.second;
	}
}

std::vector<Motif> StrategyXor::refineByPostive(
	std::unordered_map<Motif, std::pair<int, double>>& data, const size_t gsize, const size_t topk)
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
	size_t end = min(topk, idx.size());
	for(size_t i = 0; i < end; ++i)
		res.push_back(move(idx[i].second->first));
	return res;
}

std::vector<Motif> StrategyXor::shuttfle(std::vector<Motif>& motifs)
{
	MotifCommunicator comm;
	return comm.shuffle(motifs);
}

std::vector<Motif> StrategyXor::symmetricDifference(std::vector<Motif>& a, std::vector<Motif>& b)
{
	sort(a.begin(), a.end());
	sort(b.begin(), b.end());
	vector<Motif> res;
	set_symmetric_difference(make_move_iterator(a.begin()), make_move_iterator(a.end()),
		make_move_iterator(b.begin()), make_move_iterator(b.end()), back_inserter(res));
	return res;
}

