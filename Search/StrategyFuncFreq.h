#pragma once
#include "Graph.h"
#include "Motif.h"
#include "CandidateMethod.h"
#include "StrategyBase.h"
#include "TopKHolder.hpp"
#include <vector>
#include <forward_list>

class Network;

class StrategyFuncFreq
	: public StrategyBase
{
	// input options:
	int k; // number of result
	double alpha; // penalty for negative frequency
	double minSup; // minimum show up probability among postive subjects
	double pSnap; // minimum show up probability among a subject's all snapshots
	int smin, smax; // minimum/maximum motif size

// local parameters shared by internal functions (valid during searching is called)
	int nNode;
	unsigned nMinSup;
	std::vector<double> topKScores;
	using subject_t = std::vector<Graph>;
	using dataset_t = std::vector<subject_t>;
	const std::vector<std::vector<Graph>>* pgp, *pgn;
	size_t nSubPosGlobal, nSubNegGlobal;

	// single list
	class slist : public std::forward_list<const subject_t*>
	{
		size_t _s = 0;
	public:
		size_t size() const { return _s; }
		void push(const subject_t* j) {
			push_front(j);
			++_s;
		}
		iterator eraseAfter(const_iterator position) {
			--_s;
			return erase_after(position);
		}
	};

	// parameters for distribution
	int numMotifExplored;

public:
	static const std::string name;
	static const std::string usage;

	StrategyFuncFreq() = default;

	virtual bool parse(const std::vector<std::string>& param);

	virtual std::vector<Motif> search(const Option& opt,
		const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg);

	double objectFunction(const double freqPos, const double freqNeg);
	
	// high level private functions:
private:
	std::vector<Edge> getEdges();

	std::vector<Motif> method_enum1();

	std::vector<Motif> master(Network& net);
	int slaver(Network& net);
	// implementation level private function
private:
	// pass the snapshot level test (pSnap) and subject level test (minSup)
	bool checkEdge(const int s, const int d) const;
	bool checkEdge(const int s, const int d, const std::vector<Graph>& sub) const;
	std::pair<int, int> countEdge(const int s, const int d) const;
	int countEdge(const int s, const int d, const std::vector<std::vector<Graph>>& sub) const;
	std::pair<int, int> countMotif(const Motif& m) const;
	int countMotif(const Motif& m, const std::vector<std::vector<Graph>>& subs) const;

	void removeSupport(slist& sup, std::vector<const subject_t*>& rmv, const Edge& e);

	void _enum1(const unsigned p, Motif& curr, slist& supPos, slist& supNeg,
		TopKHolder<Motif, double>& res, const std::vector<Edge>& edges);
	std::vector<Motif> _enum1_nofun(const unsigned p, Motif& curr,
		slist& supPos, slist& supNeg, const std::vector<Edge>& edges);

	std::pair<int, int> master_gather_count(Network& net, const Motif& m);
	int master_gather_count_pos(Network& net, const Motif& m);
	std::vector<Edge> master_gather_edges(Network& net);


	void slave_edge_counting(Network& net);
	int slave_motif_counting(Network& net);

	void _enum1_dis1(const unsigned p, Motif& curr, slist& supPos, slist& supNeg,
		TopKHolder<Motif, double>& res, const std::vector<Edge>& edges, Network& net);
};

