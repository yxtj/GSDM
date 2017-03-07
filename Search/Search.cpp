// Search.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../common/Graph.h"
#include "../common/Motif.h"
#include "../common/SubjectInfo.h"
#include "../net/NetworkThread.h"
#include "Option.h"
#include "StrategyFactory.h"
#include "DataHolder.h"

using namespace std;

Graph loadGraph(istream& is) {
	Graph res;
	res.readFromStream(is);
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

vector<vector<string>> loadFnList(const string& folder,
	const vector<int>& types, const int nSub, const int nSnap, const int nSkip = 0)
{
	boost::filesystem::path root(folder);
	if(!exists(root)) {
		cerr << "cannot open graph folder: " << folder << endl;
		throw invalid_argument("cannot open graph folder: " + folder);
	}
	size_t limitSub = nSub >= 0 ? nSub : numeric_limits<size_t>::max();
	size_t limitSnp = nSnap > 0 ? nSnap : numeric_limits<size_t>::max();
	unordered_set<int> validType(types.begin(), types.end());

	// sort up the file list (ensure the file order)
	map<decltype(SubjectInfo::id), vector<SubjectInfo>> id2fn;
	for(auto it = boost::filesystem::directory_iterator(root);
		it != boost::filesystem::directory_iterator(); ++it)
	{
		SubjectInfo sub;
		string fn = it->path().filename().string();
		if(boost::filesystem::is_regular_file(it->status()) && sub.parseFromFilename(fn)) {
			// check type
			if(validType.find(sub.type) == validType.end())
				continue;
			id2fn[sub.id].push_back(move(sub));
		}
	}
	if(nSkip > 0) {
		auto it = id2fn.begin();
		advance(id2fn.begin(), nSkip);
		id2fn.erase(id2fn.begin(), it);
	}

	vector<vector<string>> fnList;
	fnList.reserve(min(limitSub, id2fn.size()));
	size_t cntSub = 0;
	for(auto& sfp : id2fn) {
		if(cntSub++ >= limitSub)
			break;
//		cout <<"rank: "<<rank<< " type: " << types[0] << " id: " << sfp.first << endl;
		// sort the snapshot files
		if(sfp.second.size() > limitSnp) {
			//partial_sort(sfp.second.begin(), sfp.second.begin() + limitSnp, sfp.second.end());
			sort(sfp.second.begin(), sfp.second.end(), [](SubjectInfo& l, SubjectInfo& r) {
				return l.id < r.id ? true : l.id == r.id&&l.seqNum < r.seqNum;
			});
			//sort(sfp.second.begin(), sfp.second.end());
			sfp.second.erase(sfp.second.begin() + limitSnp, sfp.second.end());
		}
		vector<string> fns;
		fns.reserve(min(sfp.second.size(), limitSnp));
		for(auto& sub : sfp.second)
			fns.push_back(sub.genFilename());
		fnList.push_back(move(fns));
	}
	return fnList;
}

DataHolder loadData(const string& folder, const vector<vector<string>>& fnList)
{
	DataHolder res;
	for(auto& sfl : fnList) {
		Subject sub;
		for(auto& fn : sfl) {
			ifstream fin(folder + fn);
			if(!fin) {
				cerr << "cannot open file: " << fn << endl;
			}
			sub.addGraph(loadGraph(fin));
		}
		res.addSubject(move(sub));
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
	Option opt;
	if(!opt.parseInput(argc, argv)) {
		return 1;
	}

	NetworkThread::Init(argc, argv);
	NetworkThread* net = NetworkThread::GetInstance();
	int rank = net->id();
	int size = net->size();

	if(rank == 0 && opt.show) {
		cout << "MPI: \n"
			<< "  # instances: " << size << "\n"
			//<< "  Multithread level: " << mpiMTLevel
			<< endl;
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
		cout << "Strategy parameters: " << opt.stgParam << "\n"
			<< endl;
	}

	// part 2: parse the options and generate a strategy
	StrategyBase* strategy = StrategyFactory::generate(opt.getStrategyName());
	if(!strategy->parse(opt.stgParam)) {
		//MPI_Finalize();
		NetworkThread::Terminate();
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
	//vector<vector<Graph> > gPos = loadData(opt.graphFolder, opt.typePos, nPosSub, opt.nSnapshot, nPosSkip);
	DataHolder dPos = loadData(opt.graphFolder,
		loadFnList(opt.graphFolder, opt.typePos, nPosSub, opt.nSnapshot, nPosSkip));
	cout << "  # positive subjects: " << dPos.size() << endl;
	//vector<vector<Graph> > gNeg = loadData(opt.graphFolder, opt.typeNeg, nNegSub, opt.nSnapshot, nNegSkip);	
	DataHolder dNeg = loadData(opt.graphFolder,
		loadFnList(opt.graphFolder, opt.typeNeg, nNegSub, opt.nSnapshot, nNegSkip));
	cout << "  # negative subjects: " << dNeg.size() << endl;

	// part 4: search for motifs
	if(!opt.outFolder.empty() && (opt.outFolder.back() == '/' || opt.outFolder.back() == '\\')) {
		boost::filesystem::path p(opt.outFolder);
		boost::filesystem::create_directories(p);
	}
	//auto res=strategy->search(opt, gPos, gNeg);
	auto res = strategy->search(opt, dPos, dNeg);
	cout << rank << " - " << res.size() << endl;
//	if(res.empty()) {
//		cerr << "Warning: zero result is found." << endl;
//	}

	// part 5: output
	if(!res.empty()) {
		ofstream fout(opt.outFolder + "res-" + to_string(rank) + ".txt");
		outputFoundMotifs(fout, res);
		fout.close();
	}

	NetworkThread::Terminate();
	return 0;
}

