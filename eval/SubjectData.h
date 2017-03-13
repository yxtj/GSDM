#pragma once
#include <string>
#include <vector>
#include "../common/Graph.h"
#include "../common/GraphProb.h"

class SubjectData
{
public:
	int type;
	std::string id;
	std::vector<Graph> snapshots;
	GraphProb gp; // not used for some cases

	SubjectData() = default;
	SubjectData(const int type, const std::string& id);
};

