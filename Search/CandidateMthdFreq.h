#pragma once
#include "CandidateMethod.h"
#include "GraphProb.h"
#include <vector>
#include <utility>

class CandidateMthdFreq :
	public CandidateMethod
{
	int nNode; // size of original graph

	int smin, smax;// size of motif [smin, smax]
	double pMin; // minimum accept prob. of a motif on the graphs of a subject
	
	const std::vector<Graph>* gs;
	GraphProb gp;
public:
	static const std::string name;
	static const std::string usage;

	CandidateMthdFreq();

	virtual bool parse(const std::vector<std::string>& param);

	virtual std::vector<std::pair<Motif, double>> getCandidantMotifs(const std::vector<Graph> & gs);

private:
	void setMotifSize(const int smin=1, const int smax=std::numeric_limits<int>::max());
	void setGraphSet(const std::vector<Graph>& gs);
	std::vector<Edge> getEdges(const GraphProb& gp);

private:
	// enumerate edge-dfs
	std::vector<std::pair<Motif, double>> method_enum1();
	std::vector<std::pair<Motif, double>> _enum1(
		const unsigned p, const std::pair<Motif, double>& curr,
		const std::vector<Edge>& edges);

	// enumerate mask by node
	std::vector<std::pair<Motif, double>> method_enum2();
	std::vector<std::pair<Motif, double>> _enum2(
		const unsigned p, const std::pair<Motif, double>& curr,
		std::vector<bool>& used);

	// expand node (all possible edge combination each attempt)
	std::vector<std::pair<Motif, double>> method_node2_layer();
	void _node2_layer(std::vector<std::pair<Motif, double>>& res,
		const std::pair<MotifBuilder, double>& curr, const int expNode);
	
	// expand node (3 type)
	std::vector<std::pair<Motif, double>> method_node3();
	void _node3(std::vector<std::pair<Motif, double>>& closed,
		std::vector<std::pair<MotifBuilder, double>>& open,
		const std::pair<MotifBuilder, double>& curr, const int expNode);

	// expand node (one edge each attempt)
	std::vector<std::pair<Motif, double>> method_node4();
	std::vector<std::pair<MotifBuilder, double>> _node4(
		const std::pair<MotifBuilder, double>& curr, const int expNode);

	// expand by edge (add one edge to last layer in the subgraph tree)
	std::vector<std::pair<Motif, double>> method_edge1_bfs();
	std::vector<std::pair<MotifBuilder, double>> _edge1_bfs(
		const std::vector<std::pair<MotifBuilder, double>>& last, const std::vector<Edge>& edges);

	// expand by edge (add one edge to found set)
	std::vector<std::pair<Motif, double>> method_edge2_dp();
	std::vector<std::pair<MotifBuilder, double>> _edge2_dp(
		const std::vector<std::pair<MotifBuilder, double>>& last, const Edge& e);
};

