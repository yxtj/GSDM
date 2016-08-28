#pragma once
#include "TypeDef.h"
#include "../common/Graph.h"
#include <vector>
#include <string>
#include <functional>

class Corr2Graph
{
	std::function<bool(const double)> pred;
public:
	Corr2Graph() = default;
	Corr2Graph(const std::vector<std::string>& param);
	void parseParam(const std::vector<std::string>& param);

	graph_t getGraphVec(const corr_t& corr);
	Graph getGraph(const corr_t& corr);
private:
	double parseNumber(const std::string& v);
};

