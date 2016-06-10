#include "stdafx.h"
#include "MotifSelector.h"

using namespace std;

MotifSelector::MotifSelector()
{
}

MotifSelector::MotifSelector(const int nMotif, const int nSample, std::vector<double> refProb)
	: nMotif(nMotif), prob(refProb)
{
}

MotifSelector::~MotifSelector()
{
}

MotifSelectorIndependent::MotifSelectorIndependent(const int nMotif, const int nSample, std::vector<double> refProb)
	:MotifSelector(nMotif, nSample, refProb)
{
}

std::vector<int> MotifSelectorIndependent::select(default_random_engine& engin)
{
	uniform_real_distribution<double> rnd(0.0, 1.0);
	vector<int> res;
	for(int i = 0; i < nMotif; ++i) {
		if(rnd(engin) < prob[i])
			res.push_back(i);
	}
	return res;
}

MotifSelectorEnsure::MotifSelectorEnsure(const int nMotif, const int nSample, std::vector<double> refProb)
	:MotifSelector(nMotif, nSample, refProb), nLeftSample(nSample)
{
}

std::vector<int> MotifSelectorEnsure::select(std::default_random_engine & engin)
{
	if(nLeftSample <= 0)
		return std::vector<int>();
	uniform_real_distribution<double> rnd(0.0, 1.0);
	vector<int> res;
	// adjust the probability for next time
	for(int i = 0; i < nMotif; ++i) {
		if(rnd(engin) < prob[i]) {
			res.push_back(i);
			prob[i] = max(0.0, (prob[i] * nLeftSample - 1) / (nLeftSample - 1));
		} else {
			prob[i] = min(1.0, (prob[i] * nLeftSample) / (nLeftSample - 1));
		}
	}
	// finishing
	nLeftSample--;
	return res;
}
