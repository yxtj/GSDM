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
	Graph(const int n);

	size_t getnNode() const;
	size_t getnEdge() const;

	bool init();
	void readFromStream(std::istream& is);
	void readFromStream(std::istream& is, const int comLevel);
	void writeToStream(std::ostream& os, const int comLevel = 0) const;

	bool testEdge(const Edge& e) const;
	bool testEdge(const int s, const int d) const;
	bool testMotif(const Motif& m) const;
	bool testMotif(const MotifBuilder& m) const;
private:
	int checkCompressionLevel(std::istream& is);
	//data format: "id	a b c " i.e. "id\ta b c "
	void writeText(std::ostream& os) const;
	void readText(std::istream& is);
	//data format: boolean matrix
	void writeBinary(std::ostream& os) const;
	void readBinary(std::istream& is);
	//data format: not determined
	void writeCompressed(std::ostream& os, const int level) const;
	void readCompressed(std::istream& is, const int level);
};

std::ostream& operator<<(std::ostream& os, const Graph& g);
