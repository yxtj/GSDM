#pragma once
#include "CandidateMethod.h"
#include "StrategyBase.h"
#include "TopKHolder.hpp"
#include <vector>
#include <forward_list>

class Network;

class StrategyFuncFreqSP
	: public StrategyBase
{
	// input options:
	int k; // number of result
	int smin, smax; // minimum/maximum motif size
	double minSup; // minimum show up probability among postive subjects
	double pSnap; // minimum show up probability among a subject's all snapshots
	std::string objFunName; // the name for the objective function
	double alpha; // penalty for negative frequency

// local parameters shared by internal functions (valid during searching is called)
	int objFunID;
	int nNode;
	int nMinSup;
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
		void pushFront(const subject_t* j) {
			std::forward_list<const subject_t*>::push_front(j);
			++_s;
		}
		iterator eraseAfter(const_iterator position) {
			--_s;
			return std::forward_list<const subject_t*>::erase_after(position);
		}
	};

	// parameters for distribution
	int numMotifExplored;

	using objFun_t = double(StrategyFuncFreqSP::*)(double, double);
	//objFun_t objFun;
	std::function<double(double, double)> objFun;

	// shortest distance signature
	struct signature {
		std::vector<std::vector<int>> sd;
		signature(int n) : sd(n, std::vector<int>(n, 2*n)) {
			for(int i = 0; i < n; ++i)
				sd[i][i] = 0;
		}
		std::vector<std::vector<int>>::reference operator[](const int idx){ return sd[idx]; }
		std::vector<std::vector<int>>::const_reference operator[](const int idx) const { return sd[idx]; }
	};
	std::vector<signature> sigPos, sigNeg;

	struct motifSign {
		signature sd;
		std::vector<std::pair<int, int>> marker; // the modified edges
	};

public:
	static const std::string name;
	static const std::string usage;

	StrategyFuncFreqSP() = default;

	virtual bool parse(const std::vector<std::string>& param);

	virtual std::vector<Motif> search(const Option& opt,
		const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg);
	
	// high level private functions:
private:
	bool setObjFun(const std::string& name);
	std::vector<Edge> getEdges();
	void setSignature();

	std::vector<Motif> method_enum1();
	std::vector<Motif> method_edge1_bfs();

//	std::vector<Motif> master(Network& net);
//	int slave(Network& net);
	// implementation level private function
private:
	double objFun_diffP2N(const double freqPos, const double freqNeg);
	double objFun_marginP2N(const double freqPos, const double freqNeg);
	double objFun_ratioP2N(const double freqPos, const double freqNeg);
private:
	// pass the snapshot level test (pSnap) and subject level test (minSup)
	bool checkEdge(const int s, const int d, const std::vector<Graph>& sub) const;
	bool checkEdge(const int s, const int d) const;
	int countEdge(const int s, const int d, const std::vector<std::vector<Graph>>& sub) const;
	std::pair<int, int> countEdge(const int s, const int d) const;
	bool testMotif(const Motif& m, const std::vector<Graph>& sub) const;
	int countMotif(const Motif& m, const std::vector<std::vector<Graph>>& subs) const;
	std::pair<int, int> countMotif(const Motif& m) const;
private:
	void removeSupport(slist& sup, std::vector<const subject_t*>& rmv, const Edge& e);

	void _enum1(const unsigned p, Motif& curr, slist& supPos, slist& supNeg,
		TopKHolder<Motif, double>& res, const std::vector<Edge>& edges);
	std::vector<MotifBuilder> _edge1_bfs(const std::vector<MotifBuilder>& last,
		TopKHolder<Motif, double>& res, const std::vector<Edge>& edges);

private:
	signature genSignture(const std::vector<Graph>& gs, const double theta);
	std::vector<std::vector<int>> calA2AShortestDistance(const Graph& g); // all source-destination pairs

	void updateSP(motifSign& ms, const MotifBuilder& mOld, int s, int d);
	bool checkSPNecessary(const MotifBuilder& m, const motifSign& ms, const signature& ss);

private:
	//std::pair<int, int> master_gather_count(Network& net, const Motif& m);
	//int master_gather_count_pos(Network& net, const Motif& m);
	//std::vector<Edge> master_gather_edges(Network& net);


	//void slave_edge_counting(Network& net);
	//int slave_motif_counting(Network& net);

	//void _enum1_dis1(const unsigned p, Motif& curr, slist& supPos, slist& supNeg,
	//	TopKHolder<Motif, double>& res, const std::vector<Edge>& edges, Network& net);
};

