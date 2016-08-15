// Evaluator.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Option.h"
#include "ConfusionMatrix.h"
#include "Motif.h"
#include "Graph.h"
#include "SubjectData.h"
#include "Evaluator-IO.h"

using namespace std;

//evaluate part

bool testMotifOnSubject(const vector<Graph>& gs, const Motif& m, const Option& opt) {
	int thre = static_cast<int>(ceil(gs.size()*opt.thrsldMotifSub));
	int cnt = 0;
	for(auto& g : gs) {
		if(g.testMotif(m)) {
			++cnt;
			if(cnt >= thre)
				break;
		}
	}
	return cnt >= thre;
}

ConfusionMatrix evaluate(const vector<SubjectData>& gs, const Motif& m,
	const unordered_set<int>& typeNeg, const Option& opt)
{
	ConfusionMatrix cm;
	for(auto& g : gs) {
		bool predPos = testMotifOnSubject(g.snapshots, m, opt);
		if(typeNeg.find(g.type) != typeNeg.end()) { // real negative
			if(predPos)
				cm.fp++;
			else
				cm.tn++;
			//int& v = predPos ? cm.fp : cm.tn;
			//v++;
		} else { // real positive
			if(predPos)
				cm.tp++;
			else
				cm.fn++;
		}
	}
	return cm;
}

vector<ConfusionMatrix> evaluate(const vector<SubjectData>& gs, const vector<Motif>& ms, const Option& opt)
{
	vector<ConfusionMatrix> res;
	res.reserve(ms.size());
	unordered_set<int> typePos(opt.graphTypePos.begin(), opt.graphTypePos.end());
	//unordered_set<int> typeNeg(opt.graphTypeNeg.begin(), opt.graphTypeNeg.end());
	for(auto& m : ms) {
		res.push_back(evaluate(gs, m, typePos, opt));
	}
	return res;
}

template <class T>
ostream& operator<<(ostream& os, const vector<T>& vec) {
	for(auto& v : vec) {
		os << v << " ";
	}
	return os;
}

//main function

int main(int argc, char* argv[])
{
	Option opt;
	if(!opt.parseInput(argc, argv))
		return 1;

	ios::sync_with_stdio(false);
	cout << "Input Motif:\n"
		<< "  Folder: " << opt.motifPath << "\n"
		<< "  File name pattern: " << opt.motifPattern << "\n"
		<< "  # of motifs: " << opt.nMotif << "\n"
		<< "  # of skipped: " << opt.nSkipMotif << "\n";
	cout << "Input Graph:\n"
		<< "  Folder: " << opt.graphPath << "\n"
		<< "  Types of positive graph: " << opt.graphTypePos << "\n"
		<< "  Types of negative graph: " << opt.graphTypeNeg << "\n"
		<< "  # of graph: " << opt.nGraph<< "\n"
		<< "  # of skipped: " << opt.nSkipGraph<< "\n";
	cout << "Output file: " << opt.outputFile << "\n"
		<< "Log file: " << opt.logFile << endl;

	vector<SubjectData> gts;
	vector<Motif> ms;
	try {
		cout << "Loading graph data...";
		cout.flush();
		gts = loadGraph(opt.graphPath, opt.graphTypes, opt.nGraph, opt.nSkipGraph);
		cout << "  # loaded: " << gts.size() << endl;
		cout << "Loading motif data...";
		cout.flush();
		ms = loadMotif(opt.motifPath, opt.motifPattern, opt.nMotif, opt.nSkipMotif);
		cout << "  # loaded: " << ms.size() << endl;
	} catch(exception& e) {
		cerr << "load data failed!\n" << e.what() << endl;
		return 2;
	}

	cout << "Evalating..." << endl;
	vector<ConfusionMatrix> res = evaluate(gts, ms, opt);
	
	cout << "Outputing result..." << endl;
	ofstream fout(opt.outputFile);
	if(!fout) {
		cerr << "Cannot open output file: " << opt.outputFile << endl;
		return 1;
	}
	showConfusionMatrixHead(fout);
	fout << '\n';
	showConfusionMatrix(fout, res);

	return 0;
}

