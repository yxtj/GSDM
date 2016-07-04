#pragma once

#include <vector>

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
	return !(lth==rth);
}
inline bool operator<(const Edge &lth, const Edge &rth) {
	return lth.s < rth.s ? true : lth.s == rth.s && lth.d < rth.d;
}

class Motif
{
public:
	int id; // this is a redundant information to out side identifier
	int nNode, nEdge;
	std::vector<Edge> edges;
public:
	Motif()=default;
	Motif(const Motif& m) = default;
	Motif(Motif&& m);

	void autoGen(const unsigned seed, const int id, const int nEdge, const int nNodeInGraph);
	bool testExistence(const std::vector<std::vector<bool> >& aMatrix) const;
	bool testExistence(const std::vector<std::vector<int> >& aList) const;

	void addEdge(const Edge& e) { addEdge(e.s, e.d); }
	void addEdge(const int s, const int d);
	void compile();
	bool check() const;
private:
	bool checkNumber() const;
	bool checkBasicLogic() const;//order and duplication
	bool checkConnect() const;
};

bool operator==(const Motif& lth, const Motif& rth);
