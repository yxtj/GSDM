#pragma once
#include <vector>
#include <utility>
#include <map>
#include <mutex>
#include "../common/Motif.h"

class RemoteTable
{
	struct _DataBlock; // a mutex is required but it is no-copyable which makes 
	_DataBlock* pimpl;
//	std::mutex m;
//	double lowerBound;
	// motif: current-tightest-upper-bound, #-generation
//	std::map<Motif, std::pair<double, int>> table;
public:
	RemoteTable();
	RemoteTable(RemoteTable&& oth);
	~RemoteTable();

	void init(const double LB);
	// remove all the motifs with no-more than l edges
	void sortUp(const int l);
	// input a motif with a given upperbound
	void update(const Motif& m, const double newUB);
	// update bound
	void updateLowerBound(double newLB);
	// collect valid motifs to send and clear the #-generation field.
	// valid is defined as : #-generation>0 && up>=lowerBound
	std::vector<std::pair<Motif, std::pair<double, int>>> collect();

	size_t size() const;

};

