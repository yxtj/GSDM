#pragma once
#include "Graph.h"
#include "Motif.h"
#include "CandidateMethod.h"
#include "StrategyBase.h"
#include "TopKHolder.hpp"
#include <vector>
#include <forward_list>

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

	// implementation level private function
private:
	// pass the snapshot level test (pSnap) and subject level test (minSup)
	bool checkEdge(const int s, const int d) const;
	bool checkEdge(const int s, const int d, const std::vector<Graph>& sub) const;

	void removeSupport(slist& sup, std::vector<const subject_t*>& rmv, const Edge& e);

	void _enum1(const unsigned p, Motif& curr, slist& supPos, slist& supNeg,
		TopKHolder<Motif, double>& res, const std::vector<Edge>& edges);
	std::vector<Motif> _enum1_nofun(const unsigned p, Motif& curr,
		slist& supPos, slist& supNeg, const std::vector<Edge>& edges);
};

