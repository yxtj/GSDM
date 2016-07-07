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

vector<vector<Graph> > loadData(
	const string& folder, const string& fprefix, const int nSub, const int nSnap) 
{
	size_t limitSub = nSub >= 0 ? nSub : numeric_limits<size_t>::max();
	size_t limitSnp= nSnap > 0 ? nSnap : numeric_limits<size_t>::max();
	vector<vector<Graph> > res;
	if(nSub > 0)
		res.reserve(nSub);
	unordered_map<decltype(Subject::id), size_t> id2off;

	using namespace boost::filesystem;
	boost::filesystem::path root(folder);
	for(auto it = directory_iterator(root); it != directory_iterator(); ++it) {
		Subject sub;
		string fn = it->path().filename().string();
		if(fn.find(fprefix) != 0)
			continue;
		if(is_regular_file(it->status()) && checknParseGraphFilename(fn, &sub)) {
			auto jt = id2off.find(sub.id);
			if(jt == id2off.end()){
				// find a new subject
				if(res.size() >= limitSub)
					break;
				jt = id2off.emplace(sub.id, res.size()).first;
				res.push_back(vector<Graph>());
			} 
			// going to add new snapshot to existing subject
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

ostream& operator<<(ostream& os, const vector<string>& param) {
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
	cout << "Data folder prefix: " << opt.prefix << "\tGraph sub-folder: " << opt.subFolderGraph << "\n"
		<< "Nodes: " << opt.nNode << "\n"
		<< "Data parameters:\n"
		<< "  # Subject +/-: " << opt.nPosInd << " / " << opt.nNegInd << "\n"
		<< "  # snapshots: " << opt.nSnapshot << "\n"
		<< "# Motif +/-: " << opt.nPosMtf << " / " << opt.nNegMtf << "\n"
//		<< "The min prob. of a valid motif on single patient: " << opt.pMotifInd << "\n"
//		<< "The min prob. of a valid motif for all patients: " << opt.pMotifRef << "\n"
//		<< "Motif size min - max: " << opt.sMotifMin << " - " << opt.sMotifMax << "\n"
//		<< "Strategy name: " << opt.stgName << "\n"
		<< "Strategy parameters: " << opt.stgParam << "\n"
		<< "Searching method pararmeters: " << opt.mtdParam << "\n"
		<< endl;

//	vector<vector<Graph> > gPos = loadData(opt.prefix + opt.subFolderGraph + "p-", opt.nPosInd, opt.nSnapshot);
//	vector<vector<Graph> > gNeg = loadData(opt.prefix + opt.subFolderGraph + "n-", opt.nNegInd, opt.nSnapshot);
	
	vector<vector<Graph> > gPos = loadData(opt.prefix + opt.subFolderGraph, "0-", opt.nPosInd, opt.nSnapshot);
	vector<vector<Graph> > gNeg = loadData(opt.prefix + opt.subFolderGraph, "1-", opt.nNegInd, opt.nSnapshot);
	
//	printMotifProbDiff(gPos, gNeg, opt.prefix + "dig-pn-1-5.txt", opt.prefix + "probDiff.txt"); return 0;
//	test(gPos, gNeg); return 0;

	StrategyBase* strategy = StrategyFactory::generate(opt.getStrategyName());
	if(!strategy->parse(opt.stgParam)) {
		return 1;
	}
	auto out=strategy->search(opt, gPos, gNeg);
	cout << out.size() << endl;
	return 0;

	/*
//	cout << "PN mechanism" << endl;
	StrategyCandidate searcher;
//	StrategyCandidatePN searcher;
//	StrategySample searcher;
	vector<tuple<Motif, double, double>> res = searcher.search(
		gPos, gNeg, opt.sMotifMin, opt.sMotifMax, opt.stgName, *pssp, opt.topK, opt.pMotifRef);

	cout << "Found " << res.size() << " motifs" << endl;

	{
		cout << "Output result" << endl;
		ofstream fout(opt.prefix + "dig-pn.txt");
		outputFoundMotifs(fout, res);
	}
*/

	return 0;
}

