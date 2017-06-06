#pragma once
#include <vector>
#include <utility>
#include <string>
#include "../common/Graph.h"
#include "../common/Motif.h"
#include "../common/SubjectInfo.h"
#include "../eval/SubjectData.h"
#include "../eval/ConfusionMatrix.h"

Graph loadGraph(std::istream& is);

std::vector<SubjectData> loadGraph(const std::string& folder, const std::vector<int>& graphTypes, int limit, int nSkip);

void outputFoundMotifs(std::ostream& os, const Motif& m);

Motif parseMotif(const std::string& line);
std::vector<Motif> readMotif(std::istream& os);

std::vector<Motif> loadMotif(const std::string& folder, const std::string& fnPattern, int limit, int nSkip);

void showConfusionMatrixHead(std::ostream& os);
void showConfusionMatrix(std::ostream& os, const ConfusionMatrix& cm);

void showHead(std::ostream& os);
void showData(std::ostream& os, const ConfusionMatrix& cm);
