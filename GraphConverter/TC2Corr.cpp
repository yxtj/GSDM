#include "stdafx.h"
#include "TC2Corr.h"

using namespace std;

TC2Corr::TC2Corr(const std::string & method)
{
	if("pearson" == method) {
		symmetric = true;
		pfunCorr = &pearsonCorrelation;
	} else if("spearman" == method) {
		symmetric = true;
		pfunCorr = &spearmanCorrelation;
	} else if("mutual" == method || "mi" == method || "mutualInfo" == method) {
		symmetric = true;
		pfunCorr = &mutualInfomation;
	} else {
		throw invalid_argument("given correlation name is not supported");
	}
}

corr_t TC2Corr::getCorr(const tc_t& TC)
{
	// TC: list of signals of all nodes
	size_t nTime = TC.size();
	size_t nNode = nTime == 0 ? 0 : TC.front().size();
	// data: list of nodes' time series (ts -> list of signal)
	vector<vector<double>> data(nNode, vector<double>(nTime));
	for(size_t i = 0; i < nTime; ++i) {
		for(size_t j = 0; j < nNode; ++j)
			data[j][i] = TC[i][j];
	}
	
	corr_t res(nNode, vector<double>(nNode));
	for(size_t i = 0; i < nNode; ++i) {
		if(!symmetric) {
			for(size_t j = 0; j < i; ++j)
				res[i][j] = pfunCorr(data[i], data[j]);
		} else {
			for(size_t j = 0; j < i; ++j)
				res[i][j] = res[j][i];
		}
		for(size_t j = i; j < nNode; ++j)
			res[i][j] = pfunCorr(data[i], data[j]);
	}
	return res;
}

double TC2Corr::pearsonCorrelation(const std::vector<double>& a, const std::vector<double>& b)
{
	double sa(0.0), sb(0.0), sa2(0.0), sb2(0.0), sab(0.0);
	size_t size = a.size();
	for(size_t i = 0; i < size; ++i) {
		sa += a[i];
		sa2 += a[i] * a[i];
		sb += b[i];
		sb2 += b[i] * b[i];
		sab += a[i] * b[i];
	}
	double n = size;
	double down = (n*sa2 - sa*sa)*(n*sb2 - sb*sb);
	return down == 0.0 ? 0.0 : abs((n*sab - sa*sb) / sqrt(down));
}

double TC2Corr::spearmanCorrelation(const std::vector<double>& a, const std::vector<double>& b)
{
	size_t n = a.size();
	vector<int> idxa;
	idxa.reserve(n);
	for(size_t i = 0; i < n; ++i)
		idxa.push_back(i);
	vector<int> idxb=idxa;
	sort(idxa.begin(), idxa.end(), [&](const int lth, const int rth) {
		return a[lth] < a[rth];
	});
	sort(idxb.begin(), idxb.end(), [&](const int lth, const int rth) {
		return b[lth] < b[rth];
	});

	vector<double> didxa, didxb;
	didxa.reserve(n);
	didxb.reserve(n);
	for(size_t i = 0; i < n; ++i) {
		didxa.push_back(static_cast<double>(idxa[i]));
		didxb.push_back(static_cast<double>(idxb[i]));
	}
	return pearsonCorrelation(didxa, didxb);
}

double TC2Corr::mutualInfomation(const std::vector<double>& a, const std::vector<double>& b)
{
	return 0.0;
}
