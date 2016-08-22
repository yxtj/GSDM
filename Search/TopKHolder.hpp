#pragma once
#include <vector>

template<class T, typename S = double>
struct TopKHolder {
	size_t k;
	std::vector<std::pair<T, S>> data;

	TopKHolder(const size_t k);
	size_t size() const;
	bool updatable(const S s) const;
	bool update(T& m, const S s);
	S lastScore() const;
};

template<class T, typename S>
TopKHolder<T, S>::TopKHolder(const size_t k)
	:k(k)
{
	data.reserve(k);
}

template<class T, typename S>
size_t TopKHolder<T, S>::size() const
{
	return data.size();
}
template<class T, typename S>
bool TopKHolder<T, S>::updatable(const S s) const
{
	return data.size() < k || s > lastScore();
}

template<class T, typename S>
bool TopKHolder<T, S>::update(T& m, const S s)
{
	size_t p = find_if(data.rbegin(), data.rend(), [s](const pair<T, S>& p) {
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

template<class T, typename S>
S TopKHolder<T, S>::lastScore() const
{
	return data.empty() ? -numeric_limits<S>::min() : data.back().second;
}
