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

ConfusionMatrix evaluate(const vector<SubjectData>& gs, const Motif& m, const Option& opt) {
	ConfusionMatrix cm;
	for(auto& g : gs) {
		bool predPos = testMotifOnSubject(g.snapshots, m, opt);
		if(g.type == 0) { // real negative
			if(predPos)
				cm.fp++;
			else
				cm.tn++;
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
	for(auto& m : ms) {
		res.push_back(evaluate(gs, m, opt));
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
		<< "  Types of cared graph: " << opt.graphTypes<< "\n"
		<< "  # of graph: " << opt.nGraph<< "\n"
		<< "  # of skipped: " << opt.nSkipGraph<< "\n";
	cout << "Output file: " << opt.outputFile << "\n"
		<< "Log file: " << opt.logFile << endl;

	cout << "Loading graph data..." << endl;
	vector<SubjectData> gts = loadGraph(opt.graphPath, opt.graphTypes, opt.nGraph, opt.nSkipGraph);
	cout << "Loading motif data..." << endl;
	vector<Motif> ms = loadMotif(opt.motifPath, opt.motifPattern, opt.nMotif, opt.nSkipMotif);
	
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

