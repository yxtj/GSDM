#include "stdafx.h"
#include "Graph.h"

#include "power_law_distribution.hpp"

using namespace std;

Graph::Graph()
{
}


Graph::~Graph()
{
}

std::vector<std::vector<int> > Graph::matrix2list() const {
	std::vector<std::vector<int> > res;
	for(int i = 0; i < nNode; ++i) {
		vector<int> list;
		for(int j = 0; j < nNode; ++j) {
			if(adj[i][j])
				list.push_back(j);
		}
		res.push_back(move(list));
	}
	return res;
}

void Graph::genNodes(const int num)
{
	adj.resize(num,vector<bool>(num,false));
	nNode = num;
}

void Graph::genRandomEdge(default_random_engine& engin, const double alpha)
{
	power_law_distribution<double> disDeg(alpha); //result: from 1 to +inf
	uniform_int_distribution<int> disNode(0, nNode-1);
//	default_random_engine engin(seed);
	auto gen = [&]() { //from 0 to nNode-1
		return min(static_cast<int>(disDeg(engin) + 1e-8), nNode) - 1; 
	};
	for(int i = 0; i < nNode; ++i) {
		int deg = gen();
		//because we do not check redundant, the real degree may be smaller than 'deg'
		while(deg--) {
			int t;
			do {
				t = disNode(engin);
			} while(t == i);
			//redundant edges is ignored
			adj[i][t]=adj[t][i]=true;
		}
	}
}

void Graph::applyMotifs(const std::vector<Motif>& motifs, const std::vector<int>& usedMotifID)
{
	for(int id : usedMotifID) {
		for(const Edge& e : motifs[id].edges) {
			adj[e.d][e.s]=adj[e.s][e.d] = true;
		}
	}
}

void Graph::setExistedMotifs(const std::vector<Motif>& motifs)
{
	motifID.clear();
	int n = motifs.size();
	for(int i = 0; i < n; ++i) {
		if(motifs[i].testExistence(adj))
			motifID.push_back(motifs[i].id);
	}
}

void Graph::setNumEdge()
{
	int cnt = 0;
	for(auto& line : adj) {
		for(bool b : line)
			if(b)
				++cnt;
	}
	nEdge = cnt;
}

void Graph::autoGen(std::default_random_engine& engin, const int nNode, const double alpha,
	const std::vector<Motif>& motifs, const std::vector<int>& usedMotifID)
{
	genNodes(nNode);
	genRandomEdge(engin, alpha);
	applyMotifs(motifs, usedMotifID);
	setNumEdge();
	setExistedMotifs(motifs);
}

std::ostream & operator<<(std::ostream & os, const Graph & g)
{
	size_t n = g.adj.size();
	os << g.nNode << '\n';
	for(size_t i = 0; i < n;++i) {
		os << i << '\t';
		for(size_t j = 0; j < n; ++j) {
			if(g.adj[i][j])
				os << j << ' ';
		}
		os << '\n';
	}
	return os;
}
