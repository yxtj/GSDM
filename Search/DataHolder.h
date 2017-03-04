#pragma once
#include "Subject.h"
#include <vector>

class DataHolder
{
	std::vector<Subject> ss;

public:
	DataHolder();
	~DataHolder();

	void addSubject(const Subject& s);
	void addSubject(Subject&& s);
	
	size_t size() const;
	bool empty() const;

	void setTheta(const double theta);
	bool contain(const Edge& e, const double minPortion) const;
	bool contain(const Motif& m, const double minPortion) const;
	int count(const Edge& e) const;
	int count(const Motif& m) const;
};

