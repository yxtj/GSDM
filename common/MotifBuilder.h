#pragma once
#include "Motif.h"
#include <unordered_map>
#include <set>

class MotifBuilder {
public:
	std::unordered_map<int, int> nodes;
	std::set<Edge> edges;
//	std::set<Edge>::iterator plEdge;
public:
	MotifBuilder() = default;
	MotifBuilder(const Motif& m);

	Motif toMotif() const;
	// return whether new edge is added by this call
	bool addEdge(const int s, const int d);
	// return whether the edge is deleted by this call
	bool removeEdge(const int s, const int d);

	bool containNode(const int n) const;
	bool containEdge(const int s, const int d) const;
//	const Edge& lastEdge() const;

	bool connected() const;

	int getnNode() const;
	int getnEdge() const;
	size_t size() const; // same as getnEdge()
	bool empty() const;
};

bool operator==(const MotifBuilder& lth, const MotifBuilder& rth);
bool operator<(const MotifBuilder& lth, const MotifBuilder& rth);

namespace std {
	template <>
	struct hash<MotifBuilder> {
		typedef MotifBuilder argument_type;
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

