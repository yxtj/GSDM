#pragma once

#include <set>
#include <vector>
#include <unordered_set>
#include <ostream>

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

namespace std {
	template <>
	struct hash<Edge> {
		typedef Edge argument_type;
		typedef std::size_t result_type;
		result_type operator()(argument_type const& e) const {
			return hash<int>()(e.s) * 137 + hash<int>()(e.d);
		}
	};
}


class Motif
{
public:
	std::vector<Edge> edges;
public:
	Motif() = default;
	Motif(const std::set<Edge>& edges);
	Motif(const std::vector<Edge>& edges);

	// return whether new edge is added by this call
	bool addEdge(const int s, const int d);
	bool addEdgeCheck(const int s, const int d);
	// return whether the edge is deleted by this call
	bool removeEdge(const int s, const int d);

	bool containNode(const int n) const;
	bool containEdge(const int s, const int d) const;
	const Edge& lastEdge() const;

	bool connected() const;

	int getnNode() const;
	int getnEdge() const;
	size_t size() const; // same as getnEdge()
	bool empty() const;
};

bool operator==(const Motif& lth, const Motif& rth);
bool operator<(const Motif& lth, const Motif& rth);

std::ostream& operator<<(std::ostream& os, const Motif& m);

inline int Motif::getnEdge() const {
	return edges.size();
}

inline size_t Motif::size() const
{
	return edges.size();
}

inline bool Motif::empty() const
{
	return edges.empty();
}

namespace std {
	template <>
	struct hash<Motif> {
		typedef Motif argument_type;
		typedef std::size_t result_type;
		result_type operator()(argument_type const& m) const {
			hash<Edge> fun;
			size_t res = 0;
			int cnt = 0;
			for(auto it = m.edges.begin(); cnt <= 3 && it != m.edges.end(); ++it, ++cnt) {
				res = res * 23 + fun(*it);
			}
			return res;
		}
	};
}

