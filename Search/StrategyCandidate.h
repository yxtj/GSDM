#pragma once
#include "Graph.h"
#include "Motif.h"
#include "CandidateMethod.h"
#include "StrategyBase.h"
#include <vector>


class StrategyCandidate
	: public StrategyBase
{
	int k;
	double pRefine;
public:
	static const std::string name;
	static const std::string usage;

	StrategyCandidate()=default;

	virtual bool parse(const std::vector<std::string>& param);

	virtual std::vector<Motif> search(const Option& opt,
		const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg);
	
private:
	// learn candidate motifs from given raw dataset
	std::vector<std::pair<Motif, double>> candidateFromOne(CandidateMethod* method, const std::vector<Graph> & gs);

	void countMotif(std::map<Motif, std::pair<int, double>>& res, std::vector<std::pair<Motif, double>> &vec);

	std::vector<Motif> pickTopK(std::map<Motif, std::pair<int, double>>& data, const size_t gsize);

	// pick the top k motifs from given candidate set, where each one with a minimum probability
	// return <motif, prob. showing up over individuals, mean prob. of happening of single individual>
	std::vector<std::tuple<Motif, double, double>> refineByAll(
		const std::vector<std::vector<std::pair<Motif,double>>>& motifs);
};

