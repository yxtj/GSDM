#pragma once
#include <vector>
#include <ostream>

// data format:
// first line: one integer indicating number of lines of this file
// rest lines: "id	a b c " i.e. "id\ta b c ", where a, b, c are dst of source id
std::vector<std::vector<int>> readGraph(const std::string& fn);
void writeGraph(std::ostream& os, const std::vector<std::vector<int>>& g);

