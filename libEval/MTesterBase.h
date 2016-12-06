#pragma once
#include <vector>
#include <string>
#include "SubjectData.h"
#include "../common/Graph.h"

class MTesterBase
{
	bool _hasSet = false;
public:
	MTesterBase() = default;
	
	virtual void set(const Motif& m);
	virtual void set(const std::vector<Motif>& ms);
	bool hasSet();
protected:
	void setMark();

public:
	virtual bool testSubject(const SubjectData& sub) const = 0;
	virtual ~MTesterBase();
protected:
	void checkName(const std::string& given, const std::string& required);
	void checkNumParam(const int given, const int required);
	void checkNumParam(const int given, const int requiredMin, const int requiredMax);
};

