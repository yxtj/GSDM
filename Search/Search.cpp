// Search.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Graph.h"
#include "GraphProb.h"
#include "Motif.h"
#include "Option.h"
#include "StrategyCandidate.h"
#include "StrategySample.h"
#include "CandidateMethodFactory.h"
#include "StrategyFactory.h"
#include "StrategyCandidatePN.h"
#include "CandidateMthdFreq.h"
#include "IOfunctions.h"

using namespace std;

Graph loadGraph(istream& is) {
	Graph res;
	res.loadDataFromStream(is);
	return res;
}

vector<vector<Graph> > loadData(const string& pre,const int n, const int m) {
	vector<vector<Graph> > res;
	res.reserve(n);
	for(int i = 0; i < n; ++i) {
		vector<Graph> temp;
		temp.reserve(m);
		for(int j = 0; j < m; ++j) {
			ifstream fin(pre + to_string(i) + "-" + to_string(j) + ".txt");
			temp.push_back(loadGraph(fin));
		}
		res.push_back(move(temp));
	}
	return res;
}

int getTotalSubjectNumber(const string& folder, const vector<int>& types) {
	boost::filesystem::path root(folder);
	if(!exists(root)) {
		cerr << "cannot open graph folder: " << folder << endl;
		throw invalid_argument("cannot open graph folder: " + folder);
	}
	unordered_set<int> validType(types.begin(), types.end());

	Subject sub;
	unordered_set<decltype(Subject::id)> subjects;
	for(auto it = boost::filesystem::directory_iterator(root);
		it != boost::filesystem::directory_iterator(); ++it)
	{
		string fn = it->path().filename().string();
		if(boost::filesystem::is_regular_file(it->status()) && checknParseGraphFilename(fn, &sub)) {
			// check type
			if(validType.find(sub.type) == validType.end())
				continue;
			subjects.insert(sub.id);
		}
	}
	return subjects.size();
}

vector<vector<Graph> > loadData(
	const string& folder, const vector<int>& types, const int nSub, const int nSnap, const int nSkip = 0) 
{
	boost::filesystem::path root(folder);
	if(!exists(root)) {
		cerr << "cannot open graph folder: " << folder << endl;
		throw invalid_argument("cannot open graph folder: " + folder);
	}
	size_t limitSub = nSub >= 0 ? nSub : numeric_limits<size_t>::max();
	size_t limitSnp= nSnap > 0 ? nSnap : numeric_limits<size_t>::max();
	vector<vector<Graph> > res;
	if(nSub > 0)
		res.reserve(nSub);
	unordered_map<decltype(Subject::id), size_t> id2off;
	unordered_set<int> validType(types.begin(), types.end());

	int cntSub = 0;
	for(auto it = boost::filesystem::directory_iterator(root);
		it != boost::filesystem::directory_iterator(); ++it)
	{
		Subject sub;
		string fn = it->path().filename().string();
		if(boost::filesystem::is_regular_file(it->status()) && checknParseGraphFilename(fn, &sub)) {
			// check type
			if(validType.find(sub.type) == validType.end())
				continue;
			// check subject
			auto jt = id2off.find(sub.id);
			if(jt == id2off.end()){
				// find a new subject
				if(++cntSub < nSkip)
					continue;
				if(res.size() >= limitSub)
					break;
				jt = id2off.emplace(sub.id, res.size()).first;
				res.push_back(vector<Graph>());
			} 
			// add a new snapshot to an existing subject
			if(res[jt->second].size() >= limitSnp) {
				continue;
			}
			ifstream fin(folder + fn);
			if(!fin) {
				cerr << "cannot open file: " << fn << endl;
			}
			res[jt->second].push_back(loadGraph(fin));
		}
	}
	return res;
}


void outputMotifAbstract(ostream& os, const Motif& m, const double probExist, const double avgProbOccur) {
	os << m.getnNode() << "\t" << m.getnEdge() << "\t"
		<< std::fixed << probExist << "\t"
		<< std::fixed << avgProbOccur << "\t";
	for(const Edge& e : m.edges) {
		os << "(" << e.s << "," << e.d << ") ";
	}
	os << '\n';
}

void outputFoundMotifs(ostream& os, const vector<tuple<Motif, double, double>>& res) {
	for(const tuple<Motif, double, double>& mp : res) {
		outputMotifAbstract(os, get<0>(mp), get<1>(mp), get<2>(mp));
	}
}


void outputFoundMotifs(ostream& os, const vector<Motif>& res) {
	for(const Motif& m : res) {
		os << m.getnNode() << "\t" << m.getnEdge() << "\t";
		for(const Edge& e : m.edges) {
			os << "(" << e.s << "," << e.d << ") ";
		}
		os << '\n';
	}
}


double probOnGS(const vector<vector<Graph>>& gs, const Motif& m)
{
	double pp = 0.0;
	for(auto& l : gs)
		pp += CandidateMethod::probOfMotif(m, l);
	pp /= gs.size();
	return pp;
}

void printMotifProbDiff(const vector<vector<Graph>>& gPos, const vector<vector<Graph>>& gNeg,
	const string& fnMotif, const string& fnOut)
{
	vector<Motif> motifs;
	ifstream fin(fnMotif);
	string line;
	while(getline(fin,line)) {
		size_t plast = line.find('\t') + 1;
		int n = stoi(line.substr(plast, line.find('\t', plast) - plast));
		Motif m;
		plast = line.rfind('\t');
		while(n--) {
			plast = line.find('(', plast + 1) + 1;
			size_t p = line.find(',', plast + 1);
			int s = stoi(line.substr(plast, p - plast));
			plast = p + 1;
			p = line.find(')',plast+1);
			int d = stoi(line.substr(plast, p - plast));
			m.addEdge(s, d);
			plast = p + 1;
		}
		motifs.push_back(move(m));
	}
	fin.close();

	ofstream fout(fnOut);
	for(size_t i = 0; i < motifs.size();++i) {
		fout << i << "\t" << fixed << probOnGS(gPos, motifs[i])
			<< "\t" << fixed << probOnGS(gNeg, motifs[i]) << "\n";
	}
	fout.close();
}

void test(const vector<vector<Graph>>& gPos, const vector<vector<Graph>>& gNeg)
{
	int n;
	cin >> n;
	Motif m;
	for(int i = 0; i < n; ++i) {
		int s, d;
		cin >> s >> d;
		m.addEdge(s, d);
		double pp = probOnGS(gPos, m);
		double pn = probOnGS(gNeg, m);
		cout << "prob. pos=" << pp << "\t" << "prob. neg=" << pn << endl;
	}
	
}

template<typename T>
ostream& operator<<(ostream& os, const vector<T>& param) {
	for(auto& p : param)
		os << p << " ";
	return os;
}

int main(int argc, char* argv[])
{
	StrategyFactory::init();
	CandidateMethodFactory::init();
	Option opt;
	if(!opt.parseInput(argc, argv)) {
		return 1;
	}
	MPI_Init(&argc, &argv);
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if(rank == 0) {
		cout << "Number of MPI instances: " << size << endl;
		cout << "Data folder prefix: " << opt.prefix << "\tGraph sub-folder: " << opt.subFolderGraph << "\n"
			<< "Output prefix: " << opt.subFolderOut << "\n"
			<< "Data parameters:\n"
			<< "  # Nodes: " << opt.nNode << "\n"
			<< "  # Subject +/-: " << opt.nPosInd << " / " << opt.nNegInd << "\n"
			<< "  # Snapshots: " << opt.nSnapshot << "\n"
			<< "  Type(s) of positive subject: " << opt.typePos<< "\n"
			<< "  Type(s) of negative subject: " << opt.typeNeg<< "\n"
			<< "Blacklist size: " << opt.blacklist.size() << "\n";
		if(!opt.blacklist.empty())
			cout << "  " << opt.blacklist << "\n";
		cout << "Searching method pararmeters: " << opt.mtdParam << "\n"
			<< "Strategy parameters: " << opt.stgParam << "\n"
			<< endl;
	}

	int nPosSub = opt.nPosInd, nPosSkip = 0;
	int nNegSub = opt.nNegInd, nNegSkip = 0;
	if(size != 1) {
		// need to set different number and starting point of different worker
		if(nPosSub == -1)
			nPosSub = getTotalSubjectNumber(opt.prefix + opt.subFolderGraph, opt.typePos);
		double partPos = static_cast<double>(nPosSub) / size;
		nPosSkip = static_cast<int>(floor(partPos*rank));
		nPosSub = (rank == size ? nPosSub : static_cast<int>(floor(partPos*(1 + rank)))) - nPosSkip;
		if(nNegSub == -1)
			nNegSub = getTotalSubjectNumber(opt.prefix + opt.subFolderGraph, opt.typeNeg);
		double partNeg = static_cast<double>(nNegSub) / size;
		nNegSkip = static_cast<int>(floor(partNeg*rank));
		nNegSub = (rank == size ? nNegSub : static_cast<int>(floor(partNeg*(1 + rank)))) - nNegSkip;
		if(!opt.graphFolderShared) {
			nPosSkip = nNegSkip = 0;
		}
	}
	vector<vector<Graph> > gPos = loadData(opt.prefix + opt.subFolderGraph, opt.typePos, nPosSub, opt.nSnapshot, nPosSkip);
	vector<vector<Graph> > gNeg = loadData(opt.prefix + opt.subFolderGraph, opt.typeNeg, nNegSub, opt.nSnapshot, nNegSkip);
	
	cout << "Finished loading:\n"
		<< "  # positive subjects: " << gPos.size() << "\n"
		<< "  # negative subjects: " << gNeg.size() << endl;
//	printMotifProbDiff(gPos, gNeg, opt.prefix + "dig-pn-1-5.txt", opt.prefix + "probDiff.txt"); return 0;
//	test(gPos, gNeg); return 0;

	StrategyBase* strategy = StrategyFactory::generate(opt.getStrategyName());
	if(!strategy->parse(opt.stgParam)) {
		MPI_Finalize();
		return 1;
	}
	if(!opt.subFolderOut.empty() && (opt.subFolderOut.back() == '/' || opt.subFolderOut.back() == '\\')) {
		boost::filesystem::path p(opt.prefix + opt.subFolderOut);
		boost::filesystem::create_directories(p);
	}
	auto res=strategy->search(opt, gPos, gNeg);
	cout << res.size() << endl;
	ofstream fout(opt.prefix + opt.subFolderOut + "res-" + to_string(rank) + ".txt");
	outputFoundMotifs(fout, res);
	fout.close();

	MPI_Finalize();
	return 0;
}

