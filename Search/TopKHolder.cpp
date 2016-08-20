#include "stdafx.h"
#include "TopKHolder.h"

using namespace std;

TopKHolder::TopKHolder(const size_t k)
	:k(k)
{
	data.reserve(k);
}

size_t TopKHolder::size() const
{
	return data.size();
}

bool TopKHolder::updatable(const double s) const
{
	return data.size() < k || s > lastScore();
}

bool TopKHolder::update(Motif& m, const double s)
{
	size_t p = find_if(data.rbegin(), data.rend(), [s](const pair<Motif, double>& p) {
		return p.second >= s;
	}) - data.rbegin();
	p = data.size() - p; // p is the place to insert the new value
	if(p < k) {
		if(data.size() < k)
			data.resize(data.size() + 1);
		for(size_t i = data.size() - 1; i > p; --i)
			data[i] = move(data[i - 1]);
		data[p] = make_pair(move(m), s);
		return true;
	}
	return false;
}

double TopKHolder::lastScore() const
{
	return data.empty() ? -numeric_limits<double>::min() : data.back().second;
}
