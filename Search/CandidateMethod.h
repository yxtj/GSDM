#pragma once
#include "Motif.h"
#include "Graph.h"

struct CandidateMethodParm;

class CandidateMethod
{
public:
	CandidateMethod();
	~CandidateMethod();

	virtual std::vector<std::pair<Motif, double>> getCandidantMotifs(const std::vector<Graph> & gs,
		const int smin, const int smax, const CandidateMethodParm& par) = 0;

	static double probOfMotif(const Motif& m, const std::vector<Graph>& gs);
	static double probOfMotif(const Motif& m, const int newS, const int newD, const std::vector<Graph>& gs);
};

struct CandidateMethodParm 
{
	CandidateMethodParm() = default;
	CandidateMethodParm(const CandidateMethodParm&) = default;
	CandidateMethodParm(CandidateMethodParm&&) = default;

	virtual void construct(const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg) = 0;
};
