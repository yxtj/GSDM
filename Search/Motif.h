#pragma once

#include <set>
#include <vector>
#include <unordered_set>

struct Edge {
	int s, d;
	Edge() {}
	Edge(const int a, const int b) :s(a), d(b) {
		//undirected edge : s<d
		if(s > d) std::swap(s, d);
	}
	void set(const int a, const int b) {
		s = a; d = b;
		if(s > d) std::swap(s, d);
	}
};

inline bool operator==(const Edge &lth, const Edge &rth) {
	//also consider the reverse direction
	return (lth.s == rth.s && lth.d == rth.d);
	//		|| (lth.s == rth.d && lth.d == rth.s);
}
inline bool operator!=(const Edge &lth, const Edge &rth) {
	return !(lth == rth);
}
inline bool operator<(const Edge &lth, const Edge &rth) {
	return lth.s < rth.s ? true : lth.s == rth.s && lth.d < rth.d;
}

class Motif
{
public:
	std::unordered_set<int> nodes; // this is a redundant information to edges
	std::set<Edge> edges; // kernel information
public:
	Motif() = default;
	Motif(const Motif& m) = default;
	Motif(Motif&& m) = default;

	Motif(const std::set<Edge>& edges);
	Motif(const std::vector<Edge>& edges);

	Motif& operator=(const Motif& m) = default;
	Motif& operator=(Motif&& m);

	// return whether new edge is added by this call
	bool addEdge(const int s, const int d);
	// return whether the edge is deleted by this call
	bool removeEdge(const int s, const int d);

	bool containNode(const int n) const;
	bool containEdge(const int s, const int d) const;
	const Edge& lastEdge() const;

	int getnNode() const;
	int getnEdge() const;
private:
	void sortUpEdges();
};

bool operator==(const Motif& lth, const Motif& rth);
bool operator<(const Motif& lth, const Motif& rth);


inline int Motif::getnNode() const {
	return nodes.size();
}
inline int Motif::getnEdge() const {
	return edges.size();
}
