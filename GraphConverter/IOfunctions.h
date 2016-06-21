#pragma once
#include "TypeDef.h"
#include "Option.h"
#include "Subject.h"
#include <map>
#include <iostream>
#include <string>

//---------------------------- Time Course ---------------------------------

std::multimap<Subject, tc_t> loadInputTC(const std::string& tcPath, const std::string& dataset);

//---------------------------- Correlation ---------------------------------

// correlation file name functions:
// filename format: <type>-<subject Id>-<scan Id>.txt
std::string genCorrFilename(const Subject& sub);
bool checkCorrFilename(const std::string& fn);
Subject parseCorrFilename(const std::string& fn) noexcept(false);
bool checknParseCorrFilename(const std::string& fn, Subject* pRes) noexcept;

std::multimap<Subject, corr_t> loadInputCorr(const std::string& corrPath);

corr_t readCorr(const std::string& fn);
void writeCorr(std::ostream& os, const corr_t& corr);

//---------------------------- Graph ---------------------------------

// graph file name functions:
// filename format: <type>-<subject Id>-<scan Id>.txt
std::string genGraphFilename(const Subject& sub);
bool checknParseCorrFilename(const std::string& fn, Subject* pRes) noexcept;


// data format:
// first line: one integer indicating number of lines of this file
// rest lines: "id	a b c " i.e. "id\ta b c ", where a, b, c are dst of source id
graph_t readGraph(const std::string& fn);
void writeGraph(std::ostream& os, const graph_t& g);

