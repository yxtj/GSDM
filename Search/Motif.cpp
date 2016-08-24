#include "stdafx.h"
#include "Motif.h"

using namespace std;

Motif::Motif(const std::set<Edge>& edges)
{
	this->edges.reserve(edges.size());
	for(const Edge& e : edges) {
		this->edges.push_back(e);
	}
}

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

bool Motif::removeEdge(const int s, const int d)
{
	auto it = find(edges.begin(), edges.end(), Edge{ s,d });
	if(it != edges.end()) {
		edges.erase(it);
		return true;
	}
	return false;
}

bool Motif::containNode(const int n) const
{
	for(const Edge& e : edges) {
		if(n == e.s || n == e.d)
			return true;
	}
	return false;
}

bool Motif::containEdge(const int s, const int d) const
{
	return find(edges.begin(), edges.end(), Edge{ s,d }) != edges.end();
}

const Edge & Motif::lastEdge() const
{
	return edges.back();
}

bool Motif::connected() const
{
	if(edges.empty())
		return true;
	unordered_map<int, int> color(edges.size());
	for(const Edge& e : edges) {
		color[e.s] = e.s;
		color[e.d] = e.d;
	}
	auto fun = [&color](int p) {
		vector<int> stack;
		while(color[p] != p) {
			p = color[p];
			stack.push_back(p);
		}
		for(int v : stack)
			color[v] = p;
		return p;
	};
	auto foo = [&color](int p, const int v) {
		while(color[p] != v) {
			int pn = color[p];
			color[p]= v;
			p = pn;
		}
	};
	for(const Edge& e : edges) {
		int nc = min(fun(e.s), fun(e.d));
		foo(e.s, nc);
		foo(e.d, nc);
	}
	int v = color.begin()->second;
	for(auto& p : color) {
		if(fun(p.first) != v)
			return false;
	}
	return true;
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

bool operator==(const Motif& lth, const Motif& rth) {
	return lth.edges == rth.edges;
}

bool operator<(const Motif & lth, const Motif & rth)
{
	return lth.edges < rth.edges;
}

std::ostream & operator<<(std::ostream & os, const Motif & m)
{
	os << m.getnEdge() << "\t";
	for(const Edge& e : m.edges) {
		os << "(" << e.s << "," << e.d << ") ";
	}
	return os;
}


