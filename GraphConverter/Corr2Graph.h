#pragma once
#include "TypeDef.h"
#include <vector>
#include <string>
#include <functional>

class Corr2Graph
{
	double thre;
	std::function<bool(const double)> pred;
public:
//	Corr2Graph() = default;
	Corr2Graph(const double th);
	Corr2Graph(const std::vector<std::string>& param);

	graph_t getGraph(const corr_t& corr);
private:
};

