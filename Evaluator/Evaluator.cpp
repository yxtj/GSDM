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

bool testMotifOnSubject(const vector<Graph>& gs, const Motif& m, const double threshold) {
	int thre = static_cast<int>(gs.size()*threshold);
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

ConfusionMatrix evaluate(const vector<SubjectData>& gs, const Motif& m, const double threshold) {
	ConfusionMatrix cm;
	for(auto& g : gs) {
		bool predPos = testMotifOnSubject(g.snapshots, m, threshold);
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

vector<ConfusionMatrix> evaluate(const vector<SubjectData>& gs, const vector<Motif>& ms, const double threshold)
{
	vector<ConfusionMatrix> res;
	res.reserve(ms.size());
	for(auto& m : ms) {
		res.push_back(evaluate(gs, m, threshold));
	}
	return res;
}

//main function

int main(int argc, char* argv[])
{
	Option opt;
	if(!opt.parseInput(argc, argv))
		return 1;

	ios::sync_with_stdio(false);

	vector<SubjectData> gts = loadGraph(opt.graphPath, opt.graphTypes, opt.nGraph, opt.nSkipGraph);
	vector<Motif> ms = loadMotif(opt.motifPath, opt.motifPattern, opt.nMotif, opt.nSkipMotif);

	vector<ConfusionMatrix> res = evaluate(gts, ms, opt.thrsldMotifSub);

	ofstream fout(opt.outputPath + "test_rest.txt");
	writeConfusionMatrix(fout, res);

	return 0;
}

