#pragma once
#include <vector>
#include <list>
#include <algorithm>

// hold the data with a score greater or equal to given bound
template<class T, typename S = double>
struct TopKBoundedHolder {
	const size_t k;
	S bound;
	// sorted list with decreasing (non-increasing) order of score
	std::list<std::pair<T, S>> data;

	TopKBoundedHolder(const size_t k, const S bound = worstScore());
	size_t size() const;
	static constexpr S worstScore();

	bool updatable(const S s) const;
	bool update(T&& m, const S s);
	bool update(const T& m, const S s);

	// return number of items removed by setting a new (higher) bound
	int updateBound(const S newBound);

	// return the last score in current holder
	S lastScore() const;
	// return the last score, if size()<k return - infinity
	S lastScore4Update() const;

	std::vector<T> getResult() const;
	std::vector<T> getResultMove();
	std::vector<std::pair<T, S>> getResultScore() const;
	std::vector<std::pair<T, S>> getResultScoreMove();
	std::vector<S> getScore() const;
private:
	bool _updateReal(T&& m, const S s);
};

template<class T, typename S>
TopKBoundedHolder<T, S>::TopKBoundedHolder(const size_t k, const S bound)
	:k(k), bound(bound)
{
}

template<class T, typename S>
inline size_t TopKBoundedHolder<T, S>::size() const
{
	return data.size();
}
template<class T, typename S>
constexpr S TopKBoundedHolder<T, S>::worstScore()
{
/*	if(std::is_floating_point<S>::value) {
		return nextafter(bound, std::numeric_limits<S>::lowest());
	} else {
		return bound == std::numeric_limits<S>::lowest() ? bound : bound - 1;
	}
	*/
	return std::numeric_limits<S>::lowest();
}
template<class T, typename S>
inline bool TopKBoundedHolder<T, S>::updatable(const S s) const
{
	return s > lastScore() || s == lastScore() && data.size() < k;
}

template<class T, typename S>
bool TopKBoundedHolder<T, S>::_updateReal(T&& m, const S s)
{
	auto it = std::upper_bound(data.begin(), data.end(), s, 
		[s](const S s, const std::pair<T, S>&p) {
		return s > p.second;
	});
	if(it == data.end())	return false;
	data.insert(it, make_pair(move(m), s));
	if(data.size() >= k)
		data.erase(--data.end());
	return true;
}

template<class T, typename S>
inline bool TopKBoundedHolder<T, S>::update(T&& m, const S s)
{
	if(s < bound)
		return false;
	return _updateReal(std::move(m), s);
}

template<class T, typename S>
inline bool TopKBoundedHolder<T, S>::update(const T& m, const S s)
{
	if(s < bound)
		return false;
	T temp(m);
	return _updateReal(std::move(temp), s);
}

template<class T, typename S>
inline int TopKBoundedHolder<T, S>::updateBound(const S newBound)
{
	bound = newBound;
	auto it=std::upper_bound(data.begin(), data.end(), newBound,
		[](const S v, const std::pair<T, S>&p) {
		return v > p.second;
	});
	int count = 0;
	while(it != data.end()) {
		it = data.erase(it);
		++count;
	}
	return count;
}

template<class T, typename S>
inline S TopKBoundedHolder<T, S>::lastScore() const
{
	return data.empty() ? bound : data.back().second;
}

template<class T, typename S>
inline S TopKBoundedHolder<T, S>::lastScore4Update() const
{
	return data.size() < k ? bound : data.back().second;
}

template<class T, typename S>
std::vector<T> TopKBoundedHolder<T, S>::getResult() const
{
	std::vector<T> res;
	res.reserve(data.size());
	for(auto& p : data)
		res.push_back(p.first);
	return res;
}

template<class T, typename S>
std::vector<T> TopKBoundedHolder<T, S>::getResultMove()
{
	std::vector<T> res;
	res.reserve(data.size());
	for(auto& p : data)
		res.push_back(std::move(p.first));
	return res;
}

template<class T, typename S>
inline std::vector<std::pair<T, S>> TopKBoundedHolder<T, S>::getResultScore() const
{
	std::vector<std::pair<T, S>> res;
	res.reserve(data.size());
	for(auto& p : data)
		res.push_back(p);
	return res;
}

template<class T, typename S>
inline std::vector<std::pair<T, S>> TopKBoundedHolder<T, S>::getResultScoreMove()
{
	std::vector<std::pair<T, S>> res;
	res.reserve(data.size());
	for(auto& p : data)
		res.push_back(std::move(p));
	return res;
}

template<class T, typename S>
inline std::vector<S> TopKBoundedHolder<T, S>::getScore() const
{
	std::vector<S> res;
	res.reserve(data.size());
	for(auto& p : data)
		res.push_back(p.second);
	return res;
}
