// Search.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <mpi.h>
//#include "Graph.h"
//#include "Motif.h"
#include "../common/Graph.h"
#include "../common/Motif.h"
#include "../common/SubjectInfo.h"
#include "../net/NetworkThread.h"
#include "Option.h"
#include "CandidateMethodFactory.h"
#include "StrategyFactory.h"

using namespace std;

Graph loadGraph(istream& is) {
	Graph res;
	res.readFromStream(is);
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

	SubjectInfo sub;
	unordered_set<decltype(SubjectInfo::id)> subjects;
	for(auto it = boost::filesystem::directory_iterator(root);
		it != boost::filesystem::directory_iterator(); ++it)
	{
		string fn = it->path().filename().string();
		if(boost::filesystem::is_regular_file(it->status()) && sub.parseFromFilename(fn)) {
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
	unordered_set<int> validType(types.begin(), types.end());

	// sort up the file list (ensure the file order)
	map<decltype(SubjectInfo::id), vector<string>> id2fn;
	for(auto it = boost::filesystem::directory_iterator(root);
		it != boost::filesystem::directory_iterator(); ++it)
	{
		SubjectInfo sub;
		string fn = it->path().filename().string();
		if(boost::filesystem::is_regular_file(it->status()) && sub.parseFromFilename(fn)) {
			// check type
			if(validType.find(sub.type) == validType.end())
				continue;
			id2fn[sub.id].push_back(move(fn));
		}
	}
	// load data
	vector<vector<Graph> > res(min(limitSub,id2fn.size()));
	int cntSub = 0;
	size_t pres = 0;
	for(auto& sfp : id2fn) {
		if(++cntSub <= nSkip)
			continue;
		else if(pres >= limitSub)
			break;
//		cout <<"rank: "<<rank<< " type: " << types[0] << " id: " << sfp.first << endl;
		// sort the snapshot files
		if(sfp.second.size() <= limitSnp) {
			sort(sfp.second.begin(), sfp.second.end());
		} else {
			//auto it = sfp.second.begin() + limitSnp;
			//partial_sort(sfp.second.begin(), it, sfp.second.end());
			sort(sfp.second.begin(), sfp.second.end());
		}
		vector<Graph>& vec = res[pres++];
		size_t cntSnp = 0;
		for(auto& fn : sfp.second) {
			if(++cntSnp > limitSnp)
				break;
			ifstream fin(folder + fn);
			if(!fin) {
				cerr << "cannot open file: " << fn << endl;
			}
			vec.push_back(loadGraph(fin));
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

void outputFoundMotif(ostream& os, const Motif& m) {
	os << m.getnNode() << "\t" << m.getnEdge() << "\t";
	for(const Edge& e : m.edges) {
		os << "(" << e.s << "," << e.d << ") ";
	}
}

void outputFoundMotifs(ostream& os, const vector<Motif>& res) {
	for(const Motif& m : res) {
		outputFoundMotif(os, m);
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
	// part 1: initialize
	StrategyFactory::init();
	CandidateMethodFactory::init();
	Option opt;
	if(!opt.parseInput(argc, argv)) {
		return 1;
	}
	int mpiMTLevel = 0;
/*	MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &mpiMTLevel);
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);*/

	NetworkThread::Init(argc, argv);
	NetworkThread* net = NetworkThread::GetInstance();
	int rank = net->id();
	int size = net->size();

	if(rank == 0) {
		cout << "MPI: \n"
			<< "  # instances: " << size << "\n"
			<< "  Multithread level: " << mpiMTLevel << endl;
		cout << "Data folder prefix: " << opt.prefix << "\tGraph (sub-)folder: " << opt.graphFolder << "\n"
			<< "Output folder: " << opt.outFolder << "\n"
			<< "Data parameters:\n"
			<< "  # Nodes: " << opt.nNode << "\n"
			<< "  # Subject +/-: " << opt.nPosInd << " / " << opt.nNegInd << "\n"
			<< "  # Snapshots: " << opt.nSnapshot << "\n"
			<< "  Type(s) of positive subject: " << opt.typePos << "\n"
			<< "  Type(s) of negative subject: " << opt.typeNeg << "\n"
			<< "  Part of data: " << size << "\n"
			<< "  Data in shared folder: " << boolalpha << opt.graphFolderShared << "\n"
			<< "Blacklist size: " << opt.blacklist.size() << "\n";
		if(!opt.blacklist.empty())
			cout << "  " << opt.blacklist << "\n";
		cout << "Searching method pararmeters: " << opt.mtdParam << "\n"
			<< "Strategy parameters: " << opt.stgParam << "\n"
			<< endl;
	}

	// part 2: parse the options and generate a strategy
	StrategyBase* strategy = StrategyFactory::generate(opt.getStrategyName());
	if(!strategy->parse(opt.stgParam)) {
		MPI_Finalize();
		return 1;
	}

	// part 3: load data
	if(opt.nPosInd < 0)
		opt.nPosInd = getTotalSubjectNumber(opt.graphFolder, opt.typePos);
	if(opt.nNegInd < 0)
		opt.nNegInd = getTotalSubjectNumber(opt.graphFolder, opt.typeNeg);
	int nPosSub = opt.nPosInd, nPosSkip = 0;
	int nNegSub = opt.nNegInd, nNegSkip = 0;
//	cout << "rank " << rank << " # pos " << opt.nPosInd << " # neg " << opt.nNegInd << endl;
	if(opt.holdAllData==false && size != 1) {
		// need to set different number and starting point of different worker
		double partPos = static_cast<double>(nPosSub) / size;
		nPosSkip = static_cast<int>(floor(partPos*rank));
		nPosSub = (rank == size ? nPosSub : static_cast<int>(floor(partPos*(1 + rank)))) - nPosSkip;
		double partNeg = static_cast<double>(nNegSub) / size;
		nNegSkip = static_cast<int>(floor(partNeg*rank));
		nNegSub = (rank == size ? nNegSub : static_cast<int>(floor(partNeg*(1 + rank)))) - nNegSkip;
		if(!opt.graphFolderShared) {
			nPosSkip = nNegSkip = 0;
		}
	}
	cout << "Loading graphs on rank " << rank << "..." << endl;
	vector<vector<Graph> > gPos = loadData(opt.graphFolder, opt.typePos, nPosSub, opt.nSnapshot, nPosSkip);
	cout << "  # positive subjects: " << gPos.size() << endl;
	vector<vector<Graph> > gNeg = loadData(opt.graphFolder, opt.typeNeg, nNegSub, opt.nSnapshot, nNegSkip);	
	cout << "  # negative subjects: " << gNeg.size() << endl;
//	printMotifProbDiff(gPos, gNeg, opt.prefix + "dig-pn-1-5.txt", opt.prefix + "probDiff.txt"); return 0;
//	test(gPos, gNeg); return 0;
//	return 0;

	// part 4: search for motifs
	if(!opt.outFolder.empty() && (opt.outFolder.back() == '/' || opt.outFolder.back() == '\\')) {
		boost::filesystem::path p(opt.outFolder);
		boost::filesystem::create_directories(p);
	}
	auto res=strategy->search(opt, gPos, gNeg);
	cout << res.size() << endl;
	if(res.empty()) {
		cerr << "Warning: zero result is found." << endl;
	}

	// part 5: output
	ofstream fout(opt.outFolder + "res-" + to_string(rank) + ".txt");
	outputFoundMotifs(fout, res);
	fout.close();

	NetworkThread::Terminate();
	return 0;
}

