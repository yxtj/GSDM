#pragma once
#include <vector>
#include <utility>

class DistributedTopKMaintainer
{
	// globl top-k scores <score, source>
	std::vector<std::pair<double, int>> data;
	size_t k;
public:

	void init(const int k);

	// replace the entries with the same source & sort up.
	// return the of nodes gathered
	size_t update(const std::vector<double>& recv, const int source);

	size_t size() const {
		return data.size();
	}
	bool full() const {
		return data.size() == k;
	}
	double lowest() const {
		return data.back().first;
	}

};

