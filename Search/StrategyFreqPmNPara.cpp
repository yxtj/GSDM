#include "stdafx.h"
#include <mpi.h>
#include "StrategyFreqPmNPara.h"
#include "CandidateMethodFactory.h"
#include "Option.h"
#include "../serialization/c_motif.h"

using namespace std;

const std::string StrategyFreqPmNPara::name("paraPmN");
const std::string StrategyFreqPmNPara::usage(
	"Select the common frequent motifs as result.\n"
	"Usage: " + StrategyFreqPmNPara::name + " <ratio Pos.> <acpt Neg.> <ratio Neg.>\n"
	"  <RP>: minium motif occurence among positive samples\n"
	"  <AP>: minium show-up ratio among negative snapshots, used to judge occurence or not\n"
	"  <RN>: maxium motif occurence among negative samples");

bool StrategyFreqPmNPara::parse(const std::vector<std::string>& param)
{
	try {
		checkParam(param, 3, name);
		pRefine = stod(param[1]);
		pPickNeg = stod(param[2]);
		pRefineNeg = stod(param[3]);
	} catch(exception& e) {
		cerr << e.what() << endl;
		return false;
	}
	return true;
}

std::vector<Motif> StrategyFreqPmNPara::search(const Option& opt,
	const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg)
{
	if(!checkInput(gPos, gNeg))
		return std::vector<Motif>();
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	// searching
	CandidateMethod* method = CandidateMethodFactory::generate(opt.getMethodName());
	method->parse(opt.mtdParam);

	cout << "Phase 1 (find positive):" << endl;
	unordered_map<Motif, pair<int, double>> phase1 = freqOnSet(method, gPos, opt.blacklist);
	cout << "  rank " << rank << " motifs found: " << phase1.size() << endl;
	delete method;
	
	vector<Motif> phase2;
	if(rank == 0) {
		cout << "Phase 2 (refine frequent):" << endl;
		//vector<tuple<Motif, double, double>> phase2 = refineByAll(phase1);
		size_t nList = lower_bound(opt.blacklist.begin(), opt.blacklist.end(), static_cast<int>(gPos.size())) - opt.blacklist.begin();
		phase2 = refineByPositive(phase1, gPos.size() - nList);
		cout << "  " << phase2.size() << " motifs after refinement." << endl;

		ofstream fout(opt.outFolder + "freq-pos.txt");
		for(auto& m : phase2) {
			fout << m.getnEdge() << '\t';
			for(const Edge&e : m.edges) {
				fout << '(' << e.s << ',' << e.d << ") ";
			}
			fout << '\n';
		}
		fout.close();

	} else {
	}

	MPI_Barrier(MPI_COMM_WORLD);
	cout << "Phase 3 (filter out negative frequent ones):" << endl;
	vector<Motif> phase3 = filterByNegative(phase2, gNeg);
	phase2.clear();
	cout << "  rank " << rank << " motifs after removal: "<< phase3.size() << endl;

	return phase3;
}

std::unordered_map<Motif, std::pair<int, double>> StrategyFreqPmNPara::freqOnSet(
	CandidateMethod* method, const std::vector<std::vector<Graph>>& gs, const std::vector<int>& blacklist)
{
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	size_t part = (gs.size() + size - 1) / size;
	size_t start = part*rank;
	size_t end = min(part*(rank + 1), gs.size());
	
	unordered_map<Motif, pair<int, double>> phase1;
	for(size_t i = start; i < end; ++i) {
		if(binary_search(blacklist.begin(), blacklist.end(), static_cast<int>(i)))
			continue;
		chrono::system_clock::time_point _time = chrono::system_clock::now();
		auto vec = method->getCandidantMotifs(gs[i]);
		countMotif(phase1, vec);
		auto _time_ms = chrono::duration_cast<chrono::milliseconds>(
			chrono::system_clock::now() - _time).count();
		cout << "  Rank "<< rank<<" individual " << i << " found " << vec.size()
			<< " motifs within " << _time_ms << " ms"
			<< ". All unique motifs: " << phase1.size() << endl;
	}
	// MPI merge
	constexpr int bufSize = 32 * 1024;
	char *buf = size == 1 ? nullptr : new char[bufSize];
	if(rank == 0) {
		int cntFinish = 1;
		cout << "  gathering motifs..." << endl;
		while(cntFinish != size) {
			MPI_Status st;
			MPI_Recv(buf, bufSize, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &st);
			if(st.MPI_TAG == 1) {
				cntFinish++;
			} else {
				auto mp = deserializeMP(buf).first;
				for(auto& p : mp) {
					phase1[p.first].first += p.second.first;
					phase1[p.first].second += p.second.second;
				}
			}
		}
/*		auto p = serializeMP(buf, 256, phase1.cbegin(), phase1.cend());
		cout << "bytes used: " << p.first - buf << "\t unit: " << distance(phase1.cbegin(), p.second) << endl;
		for(auto it = phase1.cbegin(); it != p.second; ++it) {
			cout<<it->second.first<<" - "<<it->second.second<<"\t" << it->first.getnEdge() << ":";
			for(const Edge& e : it->first.edges)
				cout << " (" << e.s << "," << e.d << ")";
			cout << "\n";
		}
		auto v = deserializeMP(buf);
		cout << "unit deserialized " << v.size() << endl;
		for(auto it = v.cbegin(); it != v.cend(); ++it) {
			cout << it->second.first << " - " << it->second.second << "\t" << it->first.getnEdge() << ":";
			for(const Edge& e : it->first.edges)
				cout << " (" << e.s << "," << e.d << ")";
			cout << "\n";
		}*/
	} else {
		cout << "  " << rank << " is sending motifs " << phase1.size() << endl;
		char *p;
		auto it = phase1.cbegin();
		do {
			tie(p, it) = serializeMP(buf, bufSize, it, phase1.cend());
			MPI_Send(buf, p - buf, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
		} while(it != phase1.cend());
		MPI_Send(buf, 1, MPI_CHAR, 0, 1, MPI_COMM_WORLD);
		cout << "  " << rank<< " finished sending motifs " << phase1.size() << endl;
		phase1.clear();
	}
	delete[] buf;
	MPI_Barrier(MPI_COMM_WORLD);
	return phase1;
}

void StrategyFreqPmNPara::countMotif(
	std::unordered_map<Motif, std::pair<int, double>>& res, std::vector<std::pair<Motif, double>>& vec)
{
	for(auto& p : vec) {
		auto& ref = res[p.first];
		ref.first++;
		ref.second += p.second;
	}
}

std::vector<Motif> StrategyFreqPmNPara::refineByPositive(
	std::unordered_map<Motif, std::pair<int, double>>& data, const size_t gsize, const int k)
{
	vector<decltype(data.begin())> idx;
	int minOcc = static_cast<int>(ceil(pRefine*gsize));
	auto it = data.begin();
	for(size_t i = 0; i < data.size(); ++i, ++it) {
		if(it->second.first >= minOcc) {
			it->second.second /= it->second.first;
			idx.push_back(it);
		}
	}
	sort(idx.begin(), idx.end(),
		[](const decltype(data.begin())& a, const decltype(data.begin())& b) {
		return a->second.first > b->second.first 
			|| a->second.first == b->second.first && a->second.second > b->second.second;
		//return a->second.first > b->second.first 
	});

	vector<Motif> res;
	size_t end = min(static_cast<size_t>(k), idx.size());
	for(size_t i = 0; i < end; ++i)
		res.push_back(move(idx[i]->first));
	return res;
}


std::vector<Motif> StrategyFreqPmNPara::filterByNegative(
	std::vector<Motif>& motifs,
	const std::vector<std::vector<Graph>>& gNeg)
{
	// MPI scatter
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	size_t part = (motifs.size() + size - 1) / size;
	size_t start = part*rank;
	size_t end = min(part*(rank + 1), motifs.size());

	const size_t bufSize = 32 * 1024;
	char *buf = size == 1 ? nullptr : new char[bufSize];
	if(rank == 0) {
		for(int r = 1; r < size; ++r) {
			cout << "  sending motifs to " << r << endl;
			auto it = motifs.cbegin() + min(part*r, motifs.size());
			auto itend = motifs.cbegin() + min(part*(r + 1), motifs.size());
			while(it != itend) {
				char* p;
				tie(p, it) = serializeVM(buf, bufSize, it, itend);
				MPI_Send(buf, p - buf, MPI_CHAR, r, 0, MPI_COMM_WORLD);
			}
			MPI_Send(buf, 1, MPI_CHAR, r, 1, MPI_COMM_WORLD);
//			cout << "  finished sending motifs to " << r << endl;
		}
		motifs.erase(motifs.begin() + end, motifs.end());
	} else {
		bool finish = false;
//		cout << "  receiving motifs at " << rank << endl;
		do {
			MPI_Status st;
			MPI_Recv(buf, bufSize, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &st);
			if(st.MPI_TAG == 1) {
				finish = true;
			} else {
				auto v = deserializeVM(buf).first;
				if(motifs.empty()) {
					motifs = move(v);
				} else {
					//motifs.reserve(motifs.size() + v.size());
					for(auto& m : v)
						motifs.push_back(move(m));
				}
			}
		} while(!finish);
		cout << "  received " << motifs.size() << " motifs at " << rank << endl;
	}
	delete[] buf;

//	cout << "  " << rank << " processing " << motifs.size() << " motifs";
	// normal logic
	std::vector<Motif> res;
	const int thrshd= static_cast<int>(ceil(pRefineNeg*gNeg.size()));
	for(auto& m: motifs) {
		int cnt = 0;
		for(auto it = gNeg.begin(); cnt < thrshd && it != gNeg.end(); ++it) {
			double d = CandidateMethod::probOfMotif(m, *it);
			if(d >= pPickNeg)
				++cnt;
		}
		if(cnt < thrshd)
			res.push_back(move(m));
	}
//	cout << "  " << rank << " finished processing motifs: " << res.size();
	// MPI merge

	return res;
}
