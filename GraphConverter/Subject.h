#pragma once
#include <string>
#include <functional> // for std::hash

struct Subject
{
	std::string id;
	int type;
	int sgId; // reused as both doctor's scan id for TC, and algorithm's graph id for Corr/Graph

	Subject() = default;
	Subject(const std::string& id, const int type, const int scanNum = -1);
	Subject(std::string&& id, const int type, const int scanNum = -1);
};

inline bool operator<(const Subject& a, const Subject& b) {
	return a.id < b.id;
}

inline bool operator==(const Subject& a, const Subject& b) {
	return a.id == b.id;
}

inline bool operator<=(const Subject& a, const Subject& b) {
	return a.id <= b.id;
}

namespace std {
	template<> struct hash<Subject> {
		typedef Subject argument_type;
		typedef std::size_t result_type;
		result_type operator()(argument_type const& s) const {
			return std::hash<std::string>()(s.id);
		}
	};
}
