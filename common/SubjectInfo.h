#pragma once
#include <string>
#include <functional> // for std::hash

struct SubjectInfo
{
	std::string id;
	int type;
	int seqNum; // reused as both doctor's scan id for TC, and snapshot id for Corr/Graph

	SubjectInfo() = default;
	SubjectInfo(const std::string& filename);
	SubjectInfo(const std::string& id, const int type, const int scanNum = -1);
	SubjectInfo(std::string&& id, const int type, const int scanNum = -1);

	// filename format: <type>-<subject Id>-<snapshot Id>.txt
	std::string genFilename() const;
	bool parseFromFilename(const std::string& fn);
	static bool checkFilename(const std::string& fn);
};

inline bool operator<(const SubjectInfo& a, const SubjectInfo& b) {
	return a.id < b.id;
}

inline bool operator==(const SubjectInfo& a, const SubjectInfo& b) {
	return a.id == b.id;
}

inline bool operator<=(const SubjectInfo& a, const SubjectInfo& b) {
	return a.id <= b.id;
}

namespace std {
	template<> struct hash<SubjectInfo> {
		typedef SubjectInfo argument_type;
		typedef std::size_t result_type;
		result_type operator()(argument_type const& s) const {
			return std::hash<std::string>()(s.id);
		}
	};
}
