#include "stdafx.h"
#include "MotifBuilder.h"

using namespace std;

MotifBuilder::MotifBuilder(const Motif & m)
{
	for(const Edge& e : m.edges) {
		addEdge(e.s, e.d);
	}
}

Motif MotifBuilder::toMotif() const
{
	Motif m;
	for(auto& e : edges) {
		m.addEdge(e.s, e.d);
	}
	return m;
}

bool MotifBuilder::addEdge(const int s, const int d) {
	Edge e{ s,d };
	auto p = edges.insert(e);
	if(p.second) {
		plEdge = p.first;
		++nodes[s];
		++nodes[d];
	}
	return p.second;
}

bool MotifBuilder::removeEdge(const int s, const int d)
{
	auto it = edges.find(Edge{ s,d });
	if(it != edges.end()) {
		edges.erase(it);
		if(--nodes[s] == 0) {
			nodes.erase(s);
		}
		if(--nodes[d] == 0) {
			nodes.erase(d);
		}
		return true;
	}
	return false;
}

bool MotifBuilder::containNode(const int n) const
{
	return nodes.find(n) != nodes.end();
}

bool MotifBuilder::containEdge(const int s, const int d) const
{
	return edges.find(Edge{ s,d }) != edges.end();
}

const Edge & MotifBuilder::lastEdge() const
{
	return *plEdge;
}

bool MotifBuilder::connected() const
{
	if(edges.empty())
		return true;
	unordered_map<int, int> color(nodes.size());
	for(pair<int, int> nc : nodes)
		color[nc.first] = nc.first;
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
			color[p] = v;
			p = pn;
		}
	};
	for(const Edge& e : edges) {
		int nc = min(fun(e.s), fun(e.d));
		foo(e.s, nc);
		foo(e.d, nc);
	}
	int v = color.begin()->second;
	for(auto& nc : nodes) {
		if(fun(nc.first) != v)
			return false;
	}
	return true;
}

int MotifBuilder::getnNode() const {
	return nodes.size();
}

int MotifBuilder::getnEdge() const {
	return edges.size();
}

size_t MotifBuilder::size() const
{
	return edges.size();
}

bool MotifBuilder::empty() const
{
	return edges.empty();
}

bool operator==(const MotifBuilder& lth, const MotifBuilder& rth) {
	return lth.edges == rth.edges;
}

bool operator<(const MotifBuilder & lth, const MotifBuilder & rth)
{
	return lth.edges < rth.edges;
}

