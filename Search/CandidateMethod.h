#pragma once
#include "Motif.h"
#include "Graph.h"
#include "FactoryProductTemplate.h"
#include <string>

class CandidateMethod
	: public FactoryProductTemplate
{
public:
	virtual ~CandidateMethod() = default;

	virtual bool parse(const std::vector<std::string>& param) = 0;

	virtual std::vector<std::pair<Motif, double>> getCandidantMotifs(const std::vector<Graph> & gs) = 0;

	static double probOfMotif(const Motif& m, const std::vector<Graph>& gs);
	static double probOfMotif(const Motif& m, const int newS, const int newD, const std::vector<Graph>& gs);
};
