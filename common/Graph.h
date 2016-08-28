#pragma once
#include <vector>
#include "Motif.h"
#include "MotifBuilder.h"

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

	bool testEdge(const Edge& e) const;
	bool testEdge(const int s, const int d) const;
	bool testMotif(const Motif& m) const;
	bool testMotif(const MotifBuilder& m) const;
};

