#pragma once
#include "Motif.h"
#include "Graph.h"
#include <string>

struct CandidateMethodParam;

class CandidateMethod
{
public:
	virtual ~CandidateMethod() = default;

	void checkParam(const std::vector<std::string>& param, int reqired, const std::string& name)
		noexcept(false); // throw exception if fails

	virtual bool parse(const std::vector<std::string>& param) = 0;

	virtual std::vector<std::pair<Motif, double>> getCandidantMotifs(const std::vector<Graph> & gs,
		const int smin, const int smax, const CandidateMethodParam& par) = 0;
	virtual std::vector<std::pair<Motif, double>> getCandidantMotifs(const std::vector<Graph> & gs) = 0;

	static double probOfMotif(const Motif& m, const std::vector<Graph>& gs);
	static double probOfMotif(const Motif& m, const int newS, const int newD, const std::vector<Graph>& gs);
protected:
	void checkNumber(int required, size_t given, const std::string& name) noexcept(false); // throw exception if fails
	void checkName(const std::vector<std::string>& param, const std::string& name) noexcept(false); // throw exception if fails

};

struct CandidateMethodParam 
{
	std::string name;

	CandidateMethodParam() = default;
	CandidateMethodParam(const CandidateMethodParam&) = default;
	CandidateMethodParam(CandidateMethodParam&&) = default;

	virtual void construct(const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg) = 0;
};
