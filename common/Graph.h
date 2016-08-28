#pragma once
#include <vector>
#include <iostream>
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

	size_t getnNode() const;
	size_t getnEdge() const;

	bool init();
	void readFromStream(std::istream& is, const bool compress = false);
	void writeToStream(std::ostream& os, const bool compress = false) const;

	bool testEdge(const Edge& e) const;
	bool testEdge(const int s, const int d) const;
	bool testMotif(const Motif& m) const;
	bool testMotif(const MotifBuilder& m) const;
private:
	//data format: "id	a b c " i.e. "id\ta b c "
	void writeText(std::ostream& os) const;
	void readText(std::istream& is);
	//data format: boolean matrix
	void writeBinary(std::ostream& os) const;
	void readBinary(std::istream& is);
};

std::ostream& operator<<(std::ostream& os, const Graph& g);
