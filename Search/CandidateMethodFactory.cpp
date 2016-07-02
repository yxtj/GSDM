#include "stdafx.h"
#include "CandidateMethodFactory.h"
// include conclude strategies:
#include "CandidateMthdFreq.h"
//#include "CandidateMthdInfreq.h"

using namespace std;

std::map<std::string, CandidateMethodFactory::createFun> CandidateMethodFactory::cont{};

bool CandidateMethodFactory::isValid(const std::string & name)
{
	return cont.find(name) != cont.end();
}

void CandidateMethodFactory::init()
{
	registerStrategy<CandidateMthdFreq>();
	// TODO: add new strategy here
//	registerStrategy<CandidateMthdInfreq>();
}

CandidateMethod * CandidateMethodFactory::generate(const std::string & methodName)
{
	CandidateMethod* res = nullptr;
	if(isValid(methodName)) {
		res = cont.at(methodName)();
	}
	return res;
}
