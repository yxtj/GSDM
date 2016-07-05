#include "stdafx.h"
#include "CandidateMethod.h"

using namespace std;


void CandidateMethod::checkNumber(int required, size_t given, const std::string& name)
{
	if(given != required + 1) {
		throw invalid_argument("Wrong number of sub-parameters for searching method: "
			+ name + ". " + to_string(required + 1) + " required, but "
			+ to_string(given) + " is given.");
	}
}

void CandidateMethod::checkName(const std::vector<string>& param, const std::string & name) noexcept(false)
{
	if(param[0] != name) {
		throw invalid_argument("The first searching method parameter (" + param.front()
			+ ") do not match current method name (" + name + ")");
	}
}


void CandidateMethod::checkParam(const std::vector<string>& param, int reqired, const std::string & name) noexcept(false)
{
	checkNumber(reqired, param.size(), name);
	checkName(param, name);
}

double CandidateMethod::probOfMotif(const Motif & m, const std::vector<Graph>& gs)
{
	int cnt = 0;
	for(const Graph& g : gs) {
		if(g.testMotif(m))
			++cnt;
	}
	return static_cast<double>(cnt) / gs.size();
}

double CandidateMethod::probOfMotif(const Motif & m, const int newS, const int newD, const std::vector<Graph>& gs)
{
	Motif t(m);
	t.addEdge(newS, newD);
	return probOfMotif(t, gs);
}
