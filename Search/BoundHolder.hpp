#pragma once
#include <vector>
#include <list>

// holder the data with a score greater or equal to given bound
template<class T, typename S = double>
struct BoundedHolder {
	S bound;
	std::list<std::pair<T, S>> data;

	BoundedHolder(const S bound);
	size_t size() const;
	static constexpr S worstScore();

	bool updatable(const S s) const;
	bool update(T&& m, const S s);
	bool update(const T& m, const S s);

	int updateBound(const S newBound);

	// return the last score in current holder
	S lastScore() const;
	// return the last score, if size()<k return - infinity
	S lastScore4Update() const;

	std::vector<T> getResult() const;
	std::vector<T> getResultMove();
protected:
	bool _updateReal(T&& m, const S s);
};

template<class T, typename S>
BoundedHolder<T, S>::BoundedHolder(const S bound)
	:bound(bound)
{
}

template<class T, typename S>
inline size_t BoundedHolder<T, S>::size() const
{
	return data.size();
}
template<class T, typename S>
constexpr S BoundedHolder<T, S>::worstScore()
{
	if(std::is_floating_point<S>::value) {
		return nextafter(bound, std::numeric_limits<S>::lowest());
	} else {
		return bound == std::numeric_limits<S>::lowest() ? bound : bound - 1;
	}
}
template<class T, typename S>
inline bool BoundedHolder<T, S>::updatable(const S s) const
{
	return s >= bound;
}

template<class T, typename S>
bool BoundedHolder<T, S>::_updateReal(T&& m, const S s)
{
	auto it = std::upper_bound(data.begin(), data.end(), s, [s](const std::pair<T, S>&p) {
		return p.second <= newBound;
	});
	if(it == data.end())
		return false;
	data.insert(it, make_pair(move(m), s));
	return true;
}

template<class T, typename S>
inline bool BoundedHolder<T, S>::update(T&& m, const S s)
{
	return _updateReal(std::move(m), s);
}

template<class T, typename S>
inline bool BoundedHolder<T, S>::update(const T& m, const S s)
{
	T temp(m);
	return _updateReal(std::move(temp), s);
}

template<class T, typename S>
inline int BoundedHolder<T, S>::updateBound(const S newBound)
{
	auto it = std::upper_bound(data.begin(), data.end(), s, [newBound](const std::pair<T, S>&p) {
		return p.second <= newBound;
	});
	int count = 0;
	while(it != data.end())
		it = data.erase(it);
	return count;
}

template<class T, typename S>
inline S BoundedHolder<T, S>::lastScore() const
{
	return data.empty() ? worstScore() : data.back().second;
}

template<class T, typename S>
inline S BoundedHolder<T, S>::lastScore4Update() const
{
	return data.size() < k ? worstScore() : data.back().second;
}

template<class T, typename S>
std::vector<T> BoundedHolder<T, S>::getResult() const
{
	std::vector<T> res;
	res.reserve(data.size());
	for(auto& p : data)
		res.push_back(p.first);
	return res;
}

template<class T, typename S>
std::vector<T> BoundedHolder<T, S>::getResultMove()
{
	std::vector<T> res;
	res.reserve(data.size());
	for(auto& p : data)
		res.push_back(std::move(p.first));
	return res;
}

