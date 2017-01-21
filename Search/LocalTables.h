#pragma once
#include <vector>
#include <utility>
#include <map>
#include <mutex>
#include <functional>
#include "../common/Motif.h"

class LocalTables
{
	std::vector<std::mutex> mcts;
	std::mutex mat;
	double lowerBound;
	std::function<int(const Motif&)> fnGetNParents;
public:
	// motif: current-tightest-upper-bound, #-generation-left-until-activated
	typedef std::map<Motif, std::pair<double, int>> CT_t;
	// motif, current-tightest-upper-bound
	typedef	std::list<std::pair<Motif, double>> AT_t;

	std::vector<CT_t> candidateTables;
	AT_t activatedTable;

	// initialize the lower-bound and the function for estimating a motif's num. of parents
	void init(std::function<int(const Motif&)> fn, double LB=std::numeric_limits<double>::lowest());
	// input a motif with a given upperbound, put into AT if ub>lowerBound & #left<=0
	void update(const Motif& m, const double newUB, const int num = 1);
	// remove all the motifs with no-more than l edges
	void sortUp(const int l);
	// update bound
	int updateLowerBound(double newLB);
	// is there any activated motif
	bool emptyActivated();
	// pick one activated motif
	std::pair<bool, std::pair<Motif, double>> getOne();
};

