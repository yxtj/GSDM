#include "stdafx.h"
#include "TesterIndividual.h"

using namespace std;

const std::string TesterIndividual::name("individual");
const std::string TesterIndividual::usage("Test motif one by one. Parameters:\n"
	"  freq <th>: motif shows up among no less than <th> percent of snapshots.\n"
	"  prob <th>: the probability of motif (product of P(edge)) is no less than <th>. Assume edges are independent."
);

TesterIndividual::TesterIndividual()
{
}

TesterIndividual::TesterIndividual(const std::vector<std::string>& params)
{
	int nParam = params.size();
	checkNumParam(nParam, 3);
	checkName(name, params[0]);
	if(!parse(params))
		throw invalid_argument("Cannot parse the parameters of " + name);
}

void TesterIndividual::set(const Motif & m)
{
	this->m = m;
	setMark();
}

bool TesterIndividual::testSubject(const SubjectData& sub)
{
	if(type == Type::FREQ) {
		return _testFreq(sub.snapshots);
	} else if(type == Type::PROB) {
		return _testProb(sub.snapshots);
	}
	return false;
}


TesterIndividual::~TesterIndividual()
{
}

bool TesterIndividual::parse(const std::vector<std::string>& params)
{
	if(params[1] == "freq") {
		type = Type::FREQ;
		thre = stod(params[2]);
	} else if(params[1] == "prob") {
		type = Type::PROB;
		thre = stod(params[2]);
	} else {
		return false;
	}
	return true;
}

bool TesterIndividual::_testFreq(const std::vector<Graph>& gs)
{
	int cnt = 0;
	int limit = static_cast<int>(ceil(gs.size()*thre));
	for(auto& g : gs) {
		if(g.testMotif(m)) {
			if(++cnt >= limit)
				return true;
		}
	}
	return false;
}

bool TesterIndividual::_testProb(const std::vector<Graph>& gs)
{
	// TODO: add the probability based testing
	return false;
}
