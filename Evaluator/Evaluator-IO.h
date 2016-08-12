#pragma once
#include <vector>
#include <utility>
#include <string>
#include "Graph.h"
#include "Motif.h"
#include "Subject.h"
#include "SubjectData.h"
#include "ConfusionMatrix.h"

Graph loadGraph(std::istream& is);

std::vector<SubjectData> loadGraph(const std::string& folder, const std::vector<int>& graphTypes, int limit, int nSkip);

void outputFoundMotifs(std::ostream& os, const Motif& m);

Motif readMotif(std::istream& os);

std::vector<Motif> loadMotif(const std::string& folder, const std::string& fnPattern, int limit, int nSkip);

void writeConfusionMatrix(std::ostream& os, const std::vector<ConfusionMatrix>& cm);
