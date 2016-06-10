// Search.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Graph.h"
#include "GraphProb.h"
#include "Motif.h"
#include "Option.h"
#include "Searcher.h"
#include "SearchStrategyFactory.h"
#include "SearcherFreqPN.h"
#include "StrategyFreq.h"

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

void test() {
//	StrategyFreqPara sfp;
//	sfp.pMin = 0.5;
//	StrategyFreq sf;
//	sf.motifOnIndTopK(vector<Graph>(), 10, 1, 3, sfp);

}

int main(int argc, char* argv[])
{
	Option opt;
	if(!opt.parseInt(argc, argv)) {
		return 1;
	}
	cout << "Data folder prefix: " << opt.prefix <<"\tGraph sub-folder: " << opt.subFolderGraph
		<< "\nNodes: " << opt.nNode
		<< "\nData folder prefix: "
		<< "\n# Individual +/-: " << opt.nPosInd << " - " << opt.nNegInd
		<< "\t# snapshots: " << opt.nSnapshot
		<< "\n# Motif +/-: " << opt.nPosMtf << " - " << opt.nNegMtf
		<< "\nThe min prob. of a valid motif on single patient: " << opt.pMotifInd
		<< "\nThe min prob. of a valid motif for all patients: " << opt.pMotifRef
		<< "\nMotif size min/max: " << opt.sMotifMin << " - " << opt.sMotifMax
		<< "\nStrategy name: "<<opt.stgName
		<< "\nTop K: "<<opt.topK
		<< endl;

//	test()
//	return 0;

	vector<vector<Graph> > gPos = loadData(opt.prefix + opt.subFolderGraph + "p-", opt.nPosInd, opt.nSnapshot);
	vector<vector<Graph> > gNeg = loadData(opt.prefix + opt.subFolderGraph + "n-", opt.nNegInd, opt.nSnapshot);
	
	SearchStrategyFactory::init();
	Searcher searcher;
	SearchStrategyPara* pssp = nullptr;
	if(opt.stgName == StrategyFreq::name) {
		StrategyFreqPara* p=new StrategyFreqPara();
		p->pMin = opt.pMotifInd;
		pssp = p;
	}
	/*
	vector<tuple<Motif, double, double>> res = searcher.search(
		gPos, gNeg, opt.sMotifMin, opt.sMotifMax, opt.stgName, *pssp, opt.topK, opt.pMotifRef);

	cout << "Found " << res.size() << " motifs" << endl;

	{
		cout << "Output result" << endl;
		ofstream fout(opt.prefix + "dig-p.txt");
		outputFoundMotifs(fout, res);
	}
	*/
	// next part
	cout << "PN mechanism" << endl;
	SearcherFreqPN searcher2;
	vector<tuple<Motif, double, double>> res = searcher2.search(
		gPos, gNeg, opt.sMotifMin, opt.sMotifMax, opt.stgName, *pssp, opt.topK, opt.pMotifRef);

	cout << "Found " << res.size() << " motifs" << endl;

	{
		cout << "Output result" << endl;
		ofstream fout(opt.prefix + "dig-pn.txt");
		outputFoundMotifs(fout, res);
	}



    return 0;
}

