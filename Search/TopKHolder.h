#pragma once
#include <vector>
#include "Motif.h"

//template<int k, class T, typename S=double>
struct TopKHolder {
	size_t k;
	std::vector<std::pair<Motif, double>> data;

	TopKHolder(const size_t k);
	bool updatable(const double s) const;
	bool update(Motif& m, const double s);
	double lastScore() const;
};
