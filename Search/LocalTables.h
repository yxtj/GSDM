#pragma once
#include <utility>
#include <vector>
#include <list>
#include <map>
#include <mutex>
#include <functional>
#include "../common/Motif.h"

class LocalTables
{
	// motif: current-tightest-upper-bound, #-generation-left-until-activated
	typedef std::map<Motif, std::pair<double, int>> CT_t;
	std::vector<CT_t> candidateTables; // one for each level
	mutable std::mutex mct; // lock mct first then mat, if they both should be locked

	// motif, current-tightest-upper-bound
	typedef	std::list<std::pair<Motif, double>> AT_t;
	AT_t activatedTable;
	mutable std::mutex mat; // lock mct first then mat, if they both should be locked
	std::vector<int> nActLevel; // num. of activated motifs in each level
	std::vector<int> nActLevelTotal; // num. of motifs which had once being active at each level

	double lowerBound;
	std::function<int(const Motif&)> fnGetNParents;

public:
	// initialize the lower-bound and the function for estimating a motif's num. of parents
	void init(std::function<int(const Motif&)> fn, double LB=std::numeric_limits<double>::lowest());
	// input a motif with a given upperbound, put into AT if ub>lowerBound & #left<=0
	void update(const Motif& m, const double newUB, const int num = 1);
	void addToActivated(const Motif& m, const double newUB);
	// remove all the motifs with no-more than l edges
	void sortUp(const int l);
	// update bound
	int updateLowerBound(double newLB);

	// pick one activated motif
	std::pair<bool, std::pair<Motif, double>> getOne();

	int mostRecentLevel() const;

	// is there any candidate motif
	bool emptyCandidate() const;
	// is there any candidate motif of a certain level
	bool emptyCandidate(const int level) const;

	// is there any activated motif
	bool emptyActivated() const;
	// is there any activated motif of a certain level
	bool emptyActivated(const int level) const;

	int getNumCandidate() const;
	std::vector<int> getNumCandidates() const;
	int getNumCandidate(const int level) const;

	int getNumActive() const;
	std::vector<int> getNumActives() const;
	int getNumActive(const int level) const;
	// the total num. of motifs had once being activated
	int getNumEverActive(const int level) const;

	bool empty() const;
	bool empty(const int level) const;

};

