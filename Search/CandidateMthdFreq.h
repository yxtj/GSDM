#pragma once
#include "CandidateMethod.h"
#include "Motif.h"
#include "Graph.h"
#include "GraphProb.h"
#include <vector>
#include <utility>
#include <functional>

struct CandidateMthdFreqParm;

class CandidateMthdFreq :
	public CandidateMethod
{
	int nNode; // size of original graph
	int smin, smax;// size of motif [smin, smax]
	
	const CandidateMthdFreqParm* par;

	const std::vector<Graph>* gs;
	GraphProb gp;
public:
	static const std::string name;
	CandidateMthdFreq();
	void setMotifSize(const int smin=1, const int smax=std::numeric_limits<int>::max());
	void setParam(const CandidateMethodParm& par);
	void setGraphSet(const std::vector<Graph>& gs);

	virtual std::vector<std::pair<Motif, double>> getCandidantMotifs(const std::vector<Graph> & gs,
		const int smin, const int smax, const CandidateMethodParm& par);
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

	// expand node by all possible edge combination
	std::vector<std::pair<Motif, double>> method_node2_layer();
	void _node2_layer(std::vector<std::pair<Motif, double>>& res,
		const std::pair<Motif, double>& curr, const int expNode);
	
	void dfsMotif3(std::vector<std::pair<Motif, double>>& closed,
		std::vector<std::pair<Motif, double>>& open,
		const std::pair<Motif, double>& curr, const int expNode,
		const std::vector<Graph>& gs, const GraphProb& gp);

	// expand node by one edge
	std::vector<std::pair<Motif, double>> method_node4();
	std::vector<std::pair<Motif, double>> _node4(
		const std::pair<Motif, double>& curr, const int expNode);

	// expand by edge
	std::vector<std::pair<Motif, double>> method_edge1();
	std::vector<std::pair<Motif, double>> _edge1(
		const std::pair<Motif, double>& curr, const int expNode);
private:
	std::vector<Edge> getEdges(const GraphProb& gp);
};

struct CandidateMthdFreqParm :
	public CandidateMethodParm
{
	double pMin;
	std::function<bool(double, double)> op_freq = std::less<double>();
	virtual void construct(const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg);
};
