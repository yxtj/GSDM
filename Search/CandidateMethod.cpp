#include "stdafx.h"
#include "CandidateMethod.h"

using namespace std;

double CandidateMethod::probOfMotif(const Motif & m, const std::vector<Graph>& gs)
{
	int cnt = 0;
	for(const Graph& g : gs) {
		if(g.testMotif(m))
			++cnt;
	}
	return static_cast<double>(cnt) / gs.size();
}

double CandidateMethod::probOfMotif(const Motif & m, const int newS, const int newD, const std::vector<Graph>& gs)
{
	Motif t(m);
	t.addEdge(newS, newD);
	return probOfMotif(t, gs);
}
