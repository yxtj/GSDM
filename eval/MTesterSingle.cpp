#include "stdafx.h"
#include "MTesterSingle.h"
#include "../common/GraphProb.h"

using namespace std;

const std::string MTesterSingle::name("single");
const std::string MTesterSingle::usage("Test motif one by one. Parameters:\n"
	"  freq <th>: motif shows up among no less than <th> percent of snapshots.\n"
	"  perd <th>: motif shows up periodically on all pieces, each piece contains 1/<th> snapshots.\n"
	"  prob <th>: the probability of motif (product of P(edge)) is no less than <th>. Assume edges are independent."
);


MTesterSingle::MTesterSingle(const std::vector<std::string>& params)
{
	int nParam = params.size();
	checkNumParam(nParam, 2);
//	checkName(name, params[0]);
	const string& method = params[0];
	double threshold = stod(params[1]);
	if(!parse(method, threshold))
		throw invalid_argument("Cannot parse the parameters of " + name);
}

MTesterSingle::MTesterSingle(const std::string & method, const double threshold)
{
	if(!parse(method, threshold))
		throw invalid_argument("Cannot parse the parameters of " + name);
}


void MTesterSingle::set(const Motif & m)
{
	this->m = m;
	setMark();
}

bool MTesterSingle::testSubject(const SubjectData& sub) const
{
	if(type == Type::FREQ) {
		return _testFreq(sub);
	} else if(type == Type::PERD) {
		return _testPeriod(sub);
	} else if(type == Type::PROB) {
		return _testProb(sub.gp);
	}
	return false;
}


bool MTesterSingle::parse(const std::string& method, const double threshold)
{
	if(method == "freq") {
		type = Type::FREQ;
		thre = threshold;
	} else if(method == "perd") {
		type = Type::PERD;
		thre = threshold;
	} else if(method == "prob") {
		type = Type::PROB;
		thre = threshold;
	} else {
		return false;
	}
	return true;
}

bool MTesterSingle::_testFreq(const SubjectData& sub) const
{
	int th = static_cast<int>(ceil(sub.size()*thre));
	return sub.contain_normal(m, th);
}

bool MTesterSingle::_testPeriod(const SubjectData& sub) const
{
	int th = static_cast<int>(ceil(sub.size()*thre));
	return sub.containByPeriod_normal(m, th);
}

bool MTesterSingle::_testProb(const GraphProb& gp) const
{
	return gp.probOfMotif(m) >= thre;
}
