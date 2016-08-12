#pragma once
#include <iostream>
#include <vector>
#include "Motif.h"

class Graph
{
public:
	int nNode = 0, nEdge = 0;
	std::vector<std::vector<bool> > matrix;
public:
	Graph() = default;
	Graph(const Graph& g) = default;
	Graph(Graph&& g);
	Graph(const int n);

	bool init();
	void loadDataFromStream(std::istream& is);

	bool testMotif(const Motif& m) const;
};

