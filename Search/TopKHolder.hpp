#pragma once
#include <vector>

template<class T, typename S = double>
struct TopKHolder {
	size_t k;
	std::vector<std::pair<T, S>> data;

	TopKHolder(const size_t k);
	size_t size() const;
	static constexpr S worstScore();

	bool updatable(const S s) const;
	bool update(T&& m, const S s);
	bool update(const T& m, const S s);

	// return the last score in current holder
	S lastScore() const;
	// return the last score, if size()<k return - infinity
	S lastScore4Update() const;

	std::vector<T> getResult() const;
	std::vector<T> getResultMove();
private:
	bool _updateReal(T&& m, const S s);
};

template<class T, typename S>
TopKHolder<T, S>::TopKHolder(const size_t k)
	:k(k)
{
	data.reserve(k);
}

template<class T, typename S>
inline size_t TopKHolder<T, S>::size() const
{
	return data.size();
}
template<class T, typename S>
constexpr S TopKHolder<T, S>::worstScore()
{
	return std::numeric_limits<S>::lowest();
}
template<class T, typename S>
inline bool TopKHolder<T, S>::updatable(const S s) const
{
	return data.size() < k || s > lastScore();
}

template<class T, typename S>
bool TopKHolder<T, S>::_updateReal(T&& m, const S s)
{
	size_t p = find_if(data.rbegin(), data.rend(), [s](const std::pair<T, S>& p) {
		return p.second >= s;
	}) - data.rbegin();
	p = data.size() - p; // p is the place to insert the new value
	if(p < k) {
		if(data.size() < k)
			data.resize(data.size() + 1);
		for(size_t i = data.size() - 1; i > p; --i)
			data[i] = std::move(data[i - 1]);
		data[p] = std::make_pair(std::move(m), s);
		return true;
	}
	return false;
}

template<class T, typename S>
inline bool TopKHolder<T, S>::update(T&& m, const S s)
{
	return _updateReal(std::move(m), s);
}

template<class T, typename S>
inline bool TopKHolder<T, S>::update(const T& m, const S s)
{
	T temp(m);
	return _updateReal(std::move(temp), s);
}

template<class T, typename S>
inline S TopKHolder<T, S>::lastScore() const
{
	return data.empty() ? worstScore() : data.back().second;
}

template<class T, typename S>
inline S TopKHolder<T, S>::lastScore4Update() const
{
	return data.size() < k ? worstScore() : data.back().second;
}

template<class T, typename S>
std::vector<T> TopKHolder<T, S>::getResult() const
{
	std::vector<T> res;
	res.reserve(data.size());
	for(auto& p : data)
		res.push_back(p.first);
	return res;
}

template<class T, typename S>
std::vector<T> TopKHolder<T, S>::getResultMove()
{
	std::vector<T> res;
	res.reserve(data.size());
	for(auto& p : data)
		res.push_back(std::move(p.first));
	return res;
}
