#pragma once
#include "TypeDef.h"

using graph_t = std::vector<std::vector<int>>;

class Corr2Graph
{
	double thre;
public:
//	Corr2Graph() = default;
	Corr2Graph(const double th);

	graph_t getGraph(const corr_t& corr);
};

