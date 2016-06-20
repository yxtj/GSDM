#pragma once
#include "TypeDef.h"
#include "Option.h"
#include "Subject.h"
#include <map>
#include <string>


std::string genCorrFilename(const Subject& sub, const int scanId);
bool checkCorrFilename(const std::string& fn);
std::pair<Subject, int> getInfoFromCorrFilename(const std::string& fn);

std::multimap<Subject, tc_t> loadInputTC(const Option& opt);
std::multimap<Subject, corr_t> loadInputCorr(const Option& opt);

corr_t _loadCorr(const std::string& fn);
