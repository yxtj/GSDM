#pragma once
#include "Motif.h"
#include <random>
#include <vector>
#include <ostream>

class Graph
{
public:
	int nNode, nEdge;
	std::vector<int> motifID; //the motifs showed up in this graph
	std::vector<std::vector<bool> > adj;//graph topology shown with adjacent matrix
public:
	Graph();
	~Graph();

	void autoGen(std::default_random_engine& engin, const int nNode, const double alpha,
		const std::vector<Motif>& motifs, const std::vector<int>& usedMotifID);
	std::vector<std::vector<int> > matrix2list() const;

private:
	void genNodes(const int num);
	void genRandomEdge(std::default_random_engine& engin, const double alpha);
	void applyMotifs(const std::vector<Motif>& motifs, const std::vector<int>& usedMotifID);
	void setExistedMotifs(const std::vector<Motif>& motifs);
	void setNumEdge();
	
	friend std::ostream& operator<<(std::ostream& os, const Graph& g);
};

std::ostream& operator<<(std::ostream& os, const Graph& g);

