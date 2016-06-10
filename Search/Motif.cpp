#include "stdafx.h"
#include "Motif.h"


void Motif::sortUpEdges()
{
	for(const Edge& e : edges) {
		nodes.insert(e.s);
		nodes.insert(e.d);
	}
}

Motif::Motif(const std::set<Edge>& edges)
{
	for(const Edge& e : edges) {
		this->edges.insert(e);
	}
	sortUpEdges();
}

Motif::Motif(const std::vector<Edge>& edges)
{
	for(const Edge& e : edges) {
		this->edges.insert(e);
	}
	sortUpEdges();
}

Motif & Motif::operator=(Motif && m)
{
	nodes = move(m.nodes);
	edges = move(m.edges);
	return *this;
}

bool Motif::addEdge(const int s, const int d) {
	Edge e{ s,d };
	if(edges.find(e) == edges.end()) {
		edges.insert(e);
		nodes.insert(s);
		nodes.insert(d);
		return true;
	}
	return false;
}

bool Motif::removeEdge(const int s, const int d)
{
	auto it = edges.find(Edge{ s,d });
	if(it != edges.end()) {
		edges.erase(it);
		sortUpEdges();
		return true;
	}
	return false;
}

bool Motif::containNode(const int n) const
{
	return nodes.find(n) != nodes.end();
}

bool Motif::containEdge(const int s, const int d) const
{
	return edges.find(Edge{ s,d }) != edges.end();
}

const Edge & Motif::lastEdge() const
{
	return *edges.rbegin();
}

bool operator==(const Motif& lth, const Motif& rth) {
	if(lth.getnNode() != rth.getnNode() || lth.getnEdge() != rth.getnEdge())
		return false;
	return lth.edges == rth.edges;
}

bool operator<(const Motif & lth, const Motif & rth)
{
	return lth.edges < rth.edges;
}


