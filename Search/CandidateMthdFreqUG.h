#pragma once
#include "CandidateMethod.h"
#include "GraphProb.h"

class CandidateMthdFreqUG :
	public CandidateMethod
{
	double pMin;
public:	
	static const std::string name;
	static const std::string usage;


	virtual bool parse(const std::vector<std::string>& param);
	virtual std::vector<std::pair<Motif, double>> getCandidantMotifs(const std::vector<Graph> & gs);

};

