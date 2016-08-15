#pragma once
#include <vector>
#include "Graph.h"

class GraphProb
{
public:
	int nNode = 0, nEdge = 0;
	int nSample = 0; // # of samples from which this GraphProb is got
	std::vector<std::vector<double> > matrix;
public:
	GraphProb() = default;
	GraphProb(const GraphProb& g) = default;
	GraphProb(GraphProb&& g) = default;
		//: nNode(g.nNode), nEdge(g.nEdge), nSample(g.nSample), matrix(g.matrix) {}
	GraphProb(const int n);
	GraphProb(const std::vector<Graph>& gs);

	std::vector<double>& operator[](const size_t offset);
	const std::vector<double>& operator[](const size_t offset) const;
	
public:
	void init(const std::vector<Graph>& gs);
	void init(const int n);

	// simple: consider nSample of g or not
	bool merge(const GraphProb& g, bool simple = false);
	bool merge(const std::vector<Graph>& gs);

	// a set of iterative accumulative calculation functions
	void startAccum(const int nNode);
	bool iterAccum(const Graph& g);
	bool iterAccum(const GraphProb& g, bool simple = false);
	void finishAccum();

	double probOfMotif(const Motif& m) const;
	double probOfMotif(const MotifBuilder& m) const;

private:
	void initMatrix();
	void countEdges();
};

