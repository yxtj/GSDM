#pragma once
#include <vector>

class MotifSelector
{
protected:
	int nMotif;
	std::vector<double> prob;
public:
	MotifSelector();
	MotifSelector(const int nMotif, const int nSample, std::vector<double> refProb);
	~MotifSelector();

	virtual std::vector<int> select(std::default_random_engine& engin) = 0;
};

// each select result is independent.
// ensure after called nSample times, the result follows binomial distribution (Bernoulli Dis.)
class MotifSelectorIndependent
	: public MotifSelector
{
public:
	MotifSelectorIndependent() = default;
	MotifSelectorIndependent(const int nMotif, const int nSample, std::vector<double> refProb);
	virtual std::vector<int> select(std::default_random_engine& engin);
};

// ensure after called nSample times, the averaged result are close to the refProb
class MotifSelectorEnsure
	: public MotifSelector
{
	int nLeftSample;
	std::vector<double> curProb;
public:
	MotifSelectorEnsure() = default;
	MotifSelectorEnsure(const int nMotif, const int nSample, std::vector<double> refProb);
	virtual std::vector<int> select(std::default_random_engine& engin);
};

