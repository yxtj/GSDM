#pragma once
#include <string>

struct Subject
{
	std::string id;
	int type;

	Subject() = default;
	Subject(const std::string& id, const int type);
	Subject(std::string&& id, const int type);
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
