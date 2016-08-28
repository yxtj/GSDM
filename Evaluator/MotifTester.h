#pragma once
#include "../common/Motif.h"
#include "../common/Graph.h"
#include "../common/GraphProb.h"
#include <vector>
#include <string>

class MotifTester
{
	const std::vector<Graph>& gs;
	GraphProb gp;
	double threshold;
	int nMin;
	using fun_t = bool (MotifTester::*)(const Motif&) const;
	fun_t fun;
public:
	static const std::string name;
	static const std::string usage;

	MotifTester(const std::vector<Graph>& gs);

	bool parse(const std::vector<std::string>& param);
	// return whether m is found in current graph
	bool test(const Motif& m) const;
private:
	bool _testFreq(const Motif& m) const;
	bool _testProb(const Motif& m) const;
};

