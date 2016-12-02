#pragma once
#include <vector>
#include <string>
#include "SubjectData.h"
#include "../common/Graph.h"

class TesterBase
{
	bool _hasSet = false;
public:
	TesterBase() = default;
	
	virtual void set(const Motif& m);
	virtual void set(const std::vector<Motif>& ms);
	bool hasSet();
	void setMark();

	virtual bool testSubject(const SubjectData& sub) = 0;
	virtual ~TesterBase();
protected:
	void checkName(const std::string& given, const std::string& required);
	void checkNumParam(const int given, const int required);
	void checkNumParam(const int given, const int requiredMin, const int requiredMax);
};

