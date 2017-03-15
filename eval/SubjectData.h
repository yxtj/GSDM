#pragma once
#include <string>
#include <vector>
#include "../common/Graph.h"
#include "../common/GraphProb.h"
#include "../holder/Subject.h"

class SubjectData
	: public Subject
{
public:
	int type;
	std::string id;
	GraphProb gp; // not used for some cases

	SubjectData() = default;
	SubjectData(const int type, const std::string& id);

	void initGP();
};

