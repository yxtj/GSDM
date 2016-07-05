#pragma once
#include "Graph.h"
#include "GraphProb.h"
#include "Motif.h"
#include "CandidateMethod.h"
#include <vector>

class StrategySample
{
public:
	StrategySample();

	std::vector<std::tuple<Motif, double, double>> search(
		const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg,
		const int smin, const int smax, const std::string& searchMethodyName, const CandidateMethodParam& par,
		const int k, const double pRefine);

	//GraphProb mergeGraphs(const std::vector<Graph>& gs);
private:
	// learn candidate motifs from given raw dataset
	std::vector<int> sampleGraphs(const std::vector<std::vector<Graph>>& gs, const double por);

	std::vector<std::pair<Motif, double>> candidateFromOne(const std::vector<Graph> & gs,
		int smin, int smax, CandidateMethod* strategy, const CandidateMethodParam& par);

	// pick the top k motifs from given candidate set, where each one with a minimum probability
	// return <motif, prob. showing up over individuals, mean prob. of happening of single individual>
	std::vector<std::tuple<Motif, double, double>> refineByAll(
		std::vector<std::vector<std::pair<Motif, double>>>& motifs, const double pRef);

	std::vector<std::tuple<Motif, double, double>> checkMotifOnAll(
		std::vector<std::tuple<Motif, double, double>>& motifs,
		const std::vector<std::vector<Graph>> & gs, const std::vector<int>& sampled,
		const double pMin, const double pRef);
	
	std::vector<std::tuple<Motif, double, double>> filterByNegative(
		const std::vector<std::tuple<Motif, double, double>>& motifs,
		const double pMax, const std::vector<std::vector<Graph>>& gNeg);

};

