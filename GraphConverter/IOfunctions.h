#pragma once
#include "TypeDef.h"
#include "Option.h"
#include "../common/SubjectInfo.h"
#include <map>
#include <iostream>
#include <string>

//---------------------------- Time Course ---------------------------------

// non-positive nSubject means all
std::multimap<SubjectInfo, tc_t> loadInputTC(
	const std::string& tcPath, const std::string& dataset, const int nSubject = -1, const int nSkip = 0);

//---------------------------- Correlation ---------------------------------

// correlation file name functions:
// filename format: <type>-<subject Id>-<scan Id>.txt
std::string genCorrFilename(const SubjectInfo& sub);
bool checkCorrFilename(const std::string& fn);
SubjectInfo parseCorrFilename(const std::string& fn) noexcept(false);
bool checknParseCorrFilename(const std::string& fn, SubjectInfo* pRes);

// non-positive nSubject means all
std::multimap<SubjectInfo, corr_t> loadInputCorr(const std::string& corrPath, const int Subject = -1, const int nSkip = 0);

corr_t readCorr(const std::string& fn);
void writeCorr(std::ostream& os, const corr_t& corr);

//---------------------------- Graph ---------------------------------

// graph file name functions:
// filename format: <type>-<subject Id>-<scan Id>.txt
std::string genGraphFilename(const SubjectInfo& sub);
bool checknParseGraphFilename(const std::string& fn, SubjectInfo* pRes);


// data format:
// first line: one integer indicating number of lines of this file
// rest lines: "id	a b c " i.e. "id\ta b c ", where a, b, c are dst of source id
graph_t readGraph(const std::string& fn);
void writeGraph(std::ostream& os, const graph_t& g);

