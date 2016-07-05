#include "stdafx.h"
#include "CandidateMethodFactory.h"

// include conclude strategies:
#include "CandidateMthdFreq.h"

using namespace std;

void CandidateMethodFactory::init()
{
	setOptName("method");
	setUsage("Use the following parameters to select ONE searching method\n");
// TODO: add new strategy here
	registerClass<CandidateMthdFreq>();
	registerUsage<CandidateMthdFreq>();
//	registerClass<CandidateMthdInfreq>();
}

CandidateMethod * CandidateMethodFactory::generate(const std::string & name)
{
	FactoryProductTemplate* p = FactoryTemplate::generate(name);
	CandidateMethod* res = dynamic_cast<CandidateMethod*>(p);
	return res;
}
