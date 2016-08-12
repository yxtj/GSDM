#include "stdafx.h"
#include "Motif.h"

using namespace std;

Motif::Motif(const std::vector<Edge>& edges)
{
	this->edges.reserve(edges.size());
	for(const Edge& e : edges) {
		this->edges.push_back(e);
	}
}

bool Motif::addEdge(const int s, const int d) {
	edges.push_back(Edge{ s,d });
	return true;
}

bool Motif::addEdgeCheck(const int s, const int d)
{
	Edge e{ s,d };
	auto it = find(edges.begin(), edges.end(), e);
	if(it != edges.end())
		return addEdge(e.s, e.d);
	return false;
}

int Motif::getnEdge() const {
	return edges.size();
}

int Motif::getnNode() const
{
	unordered_set<int> nodes;
	for(auto& e : edges) {
		nodes.insert(e.s);
		nodes.insert(e.d);
	}
	return nodes.size();
}

size_t Motif::size() const
{
	return edges.size();
}

bool Motif::empty() const
{
	return edges.empty();
}

bool operator==(const Motif& lth, const Motif& rth) {
	return lth.edges == rth.edges;
}

bool operator<(const Motif & lth, const Motif & rth)
{
	return lth.edges < rth.edges;
}


