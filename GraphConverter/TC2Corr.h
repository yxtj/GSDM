#pragma once
#include "TypeDef.h"

class TC2Corr
{
	std::string method;
public:
	TC2Corr() = default;
	TC2Corr(const std::string& method);

	corr_t getCorr(const tc_t& TC);
private:
	// implementation specific data and functions
	using pfunCorr_t= double(*)(const std::vector<double>& a, const std::vector<double>& b);
	pfunCorr_t pfunCorr;
	bool symmetric;

	static double pearsonCorrelation(const std::vector<double>& a, const std::vector<double>& b);
	static double spearmanCorrelation(const std::vector<double>& a, const std::vector<double>& b);
	static double mutualInfomation(const std::vector<double>& a, const std::vector<double>& b);
};

