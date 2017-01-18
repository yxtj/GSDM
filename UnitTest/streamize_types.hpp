#pragma once
#include <ostream>
#include <utility>
#include <vector>
#include <map>

template <class T1, class T2>
inline std::ostream& operator<<(std::ostream& os, const std::pair<T1, T2>& p) {
	return os << "(" << p.first << "," << p.second << ")";
}

template <class T>
inline std::ostream& operator<<(std::ostream& os, const std::vector<T>& cont) {
	os << "[";
	for(auto& t : cont)
		os << t << " ";
	return os << "]";
}

template <class K, class V>
inline std::ostream& operator<<(std::ostream& os, const std::map<K, V>& cont) {
	os << "{ ";
	
	for(auto&t : cont)
		os << "(" << t.first << ":" << t.second << ") ";
	return os << "}";
}
