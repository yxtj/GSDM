// Evaluator.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Option.h"
#include "ConfusionMatrix.h"
#include "Motif.h"
#include "Graph.h"
#include "SubjectData.h"
#include "MotifTester.h"
#include "Evaluator-IO.h"

using namespace std;

//evaluate part

vector<bool> testMotifs(const MotifTester& mt, const vector<Motif>& ms, const Option& opt)
{
	vector<bool> res(ms.size());
	for(size_t i = 0; i < ms.size(); ++i) {
		res[i] = mt.test(ms[i]);
	}
	return res;
}

vector<ConfusionMatrix> evaluate(const vector<SubjectData>& gs, const vector<Motif>& ms, const Option& opt)
{
	vector<ConfusionMatrix> res(ms.size());
	unordered_set<int> typePos(opt.graphTypePos.begin(), opt.graphTypePos.end());
	for(auto& g : gs) {
		MotifTester mt(g.snapshots);
		mt.parse(opt.motifTestMethod);
		vector<bool> predAsPos = testMotifs(mt, ms, opt);
		bool isPosSub = typePos.find(g.type) != typePos.end();
		if(isPosSub) {
			for(size_t i = 0; i < predAsPos.size(); ++i) {
				if(predAsPos[i])
					res[i].tp++;
				else
					res[i].fn++;
			}
		} else {
			for(size_t i = 0; i < predAsPos.size(); ++i) {
				if(predAsPos[i])
					res[i].fp++;
				else
					res[i].tn++;
			}
		}
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
		cout << "  # loaded subjects: " << gts.size() << endl;
		cout << "Loading motif data...";
		cout.flush();
		ms = loadMotif(opt.motifPath, opt.motifPattern, opt.nMotif, opt.nSkipMotif);
		cout << "  # loaded motifs: " << ms.size() << endl;
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

