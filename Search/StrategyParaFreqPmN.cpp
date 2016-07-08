#include "stdafx.h"
#include "StrategyParaFreqPmN.h"
#include "CandidateMethodFactory.h"
#include "CandidateMthdFreq.h"
#include "Option.h"

using namespace std;

const std::string StrategyParaFreqPmN::name("paraPmN");
const std::string StrategyParaFreqPmN::usage(
	"Select the common frequent motifs as result.\n"
	"Usage: " + StrategyParaFreqPmN::name + " <# of result> <ratio Pos.> <acpt Neg.> <ratio Neg.>\n"
	"  <RP>: minium motif occurence among positive samples\n"
	"  <AP>: minium show-up ratio among negative snapshots, used to judge occurence or not\n"
	"  <RN>: maxium motif occurence among negative samples\n");

bool StrategyParaFreqPmN::parse(const std::vector<std::string>& param)
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

std::vector<Motif> StrategyParaFreqPmN::search(const Option& opt,
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

// (de)seralizes

static char* serialize(char* res, const Motif& m) {
	int* pint = reinterpret_cast<int*>(res);
	*pint++ = m.getnEdge();
	for(const Edge& e : m.edges) {
		*pint++ = e.s;
		*pint++ = e.d;
	}
	return reinterpret_cast<char*>(pint);
}

static pair<Motif, char*> deserialize(char* p) {
	int* pint = reinterpret_cast<int*>(p);
	int ne = *pint++;
	Motif m;
	while(ne--) {
		int s = *pint++;
		int d = *pint++;
		m.addEdge(s, d);
	}
	return make_pair(move(m), reinterpret_cast<char*>(pint));
}

static pair<char*, unordered_map<Motif, pair<int, double>>::const_iterator> serializeMP(
	char* res, int bufSize, unordered_map<Motif, pair<int, double>>::const_iterator it,
	unordered_map<Motif, pair<int, double>>::const_iterator itend)
{
	size_t* numObj = reinterpret_cast<size_t*>(res);
	res += sizeof(size_t);
	size_t count = 0;
	for(; it != itend; ++it) {
		int n = it->first.getnEdge();
		int size = sizeof(int) + n * 2 * sizeof(int) + sizeof(int) + sizeof(double);
		if(size > bufSize)
			break;
		char *p = serialize(res, it->first);
		*reinterpret_cast<int*>(p) = it->second.first;
		*reinterpret_cast<double*>(p+sizeof(int)) = it->second.second;
		res += size;
		bufSize -= size;
		++count;
	}
	*numObj = count;
	return make_pair(res, it);
}

static unordered_map<Motif, pair<int, double>> deserializeMP(char* p) {
	size_t n = *reinterpret_cast<size_t*>(p);
	p += sizeof(size_t);
	unordered_map<Motif, pair<int, double>> res;
	while(n--) {
		auto mp = deserialize(p);
		p = mp.second;
		int count = *reinterpret_cast<int*>(p);
		p += sizeof(int);
		double prob = *reinterpret_cast<double*>(p);
		p += sizeof(double);
		res[mp.first] = make_pair(count, prob);
	}
	return res;
}

static char* serializeVM(char* res, int bufSize,
	vector<Motif>::const_iterator it, vector<Motif>::const_iterator itend) 
{
	size_t* numObj = reinterpret_cast<size_t*>(res);
	res += sizeof(size_t);
	size_t count = 0;
	for(; it != itend; ++it) {
		int n = it->getnEdge();
		int size = sizeof(int) + n * 2 * sizeof(int) + sizeof(int) + sizeof(double);
		if(size > bufSize)
			break;
		res = serialize(res, *it);
		bufSize -= size;
		++count;
	}
	*numObj = count;
	return res;
}

static vector<Motif> deserializeVM(char *p) {
	size_t n = *reinterpret_cast<size_t*>(p);
	p += sizeof(size_t);
	vector<Motif> res;
	while(n--) {
		auto mp = deserialize(p);
		res.push_back(move(mp.first));
		p = mp.second;
	}
	return res;
}

std::unordered_map<Motif, std::pair<int, double>> StrategyParaFreqPmN::freqOnSet(
	CandidateMethod* method, const std::vector<std::vector<Graph>>& gs)
{
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	size_t part = (gs.size() + size) / size;
	size_t start = part*rank;
	size_t end = min(part*(rank + 1), gs.size());
	
	unordered_map<Motif, pair<int, double>> phase1;
	for(size_t i = start; i < end; ++i) {
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
	char *buf = new char[4096];
	if(rank == 0) {
		int cntFinish = 1;
		while(cntFinish != size) {
			MPI_Status st;
			MPI_Recv(buf, 4096, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &st);
			if(st.MPI_TAG == 1)
				cntFinish++;
			else {
				auto mp = deserializeMP(buf);
				for(auto& p : mp) {
					phase1[p.first].first += p.second.first;
					phase1[p.first].second += p.second.second;
				}
			}
		}
		auto p = serializeMP(buf, 256, phase1.cbegin(), phase1.cend());
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
		}
	} else {
		char *p;
		auto it = phase1.cbegin();
		do {
			tie(p, it) = serializeMP(buf, 4096, it, phase1.cend());
			MPI_Send(buf, p - buf, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
		} while(it != phase1.cend());
		MPI_Send(buf, 1, MPI_CHAR, 0, 1, MPI_COMM_WORLD);
		phase1.clear();
	}
	delete[] buf;
	MPI_Barrier(MPI_COMM_WORLD);
	return phase1;
}

void StrategyParaFreqPmN::countMotif(
	std::unordered_map<Motif, std::pair<int, double>>& res, std::vector<std::pair<Motif, double>>& vec)
{
	for(auto& p : vec) {
		auto& ref = res[p.first];
		ref.first++;
		ref.second += p.second;
	}
}

std::vector<Motif> StrategyParaFreqPmN::pickTopK(
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


std::vector<Motif> StrategyParaFreqPmN::filterByNegative(
	std::vector<Motif>& motifs,
	const std::vector<std::vector<Graph>>& gNeg)
{
	// MPI scatter
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	// normal logic
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
	// MPI merge

	return res;
}
