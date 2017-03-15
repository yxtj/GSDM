#pragma once
#include <string>
#include <vector>

class Option
{
	struct implDesc;
	implDesc* pimpl;
public:
	bool show;

	int nSubject;
	int nSnapshot; 
	int nMotif;

	// path for input/output data folder:
	std::string pathMotif;
	std::string motifPattern;
	std::string pathGraph;
	std::vector<int> typePos, typeNeg, typeAll; // typeAll is automatically generated

	bool flgSortSubByType;
	std::string pathOutput;

public:
	Option();
	~Option();

	bool parseInput(int argc, char *argv[]);
private:
	std::string& sortUpPath(std::string& path);
	bool mergeGraphType();
};

