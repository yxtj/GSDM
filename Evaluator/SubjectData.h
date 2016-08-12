#pragma once
#include <string>
#include <vector>
#include "Graph.h"

class SubjectData
{
public:
	int type;
	std::string id;
	std::vector<Graph> snapshots;

	SubjectData();
	SubjectData(const int type, const std::string& id);
	~SubjectData();
};

