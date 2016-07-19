#pragma once
#include "Motif.h"
#include "MotifBuilder.h"
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
	static double probOfMotif(const MotifBuilder& m, const std::vector<Graph>& gs);

	static bool testProbOfMotifGE(const Motif& m, const std::vector<Graph>& gs, const double pMin);
	static bool testProbOfMotifGE(const MotifBuilder& m, const std::vector<Graph>& gs, const double pMin);

	static bool testProbOfMotifLE(const Motif& m, const std::vector<Graph>& gs, const double pMax);
	static bool testProbOfMotifLE(const MotifBuilder& m, const std::vector<Graph>& gs, const double pMax);

	std::vector<Motif> toMotif(const std::vector<MotifBuilder>& mbs);

};
