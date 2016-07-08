#include "stdafx.h"
#include "CandidateMethodFactory.h"

// include conclude strategies:
#include "CandidateMthdFreq.h"

using namespace std;

using parent_t = CandidateMethodFactory::parent_t;

template <>
const std::string parent_t::optName("method");
template <>
const std::string parent_t::usagePrefix(
	"Use the following parameters to select ONE searching method\n");

template <>
std::map<std::string, parent_t::createFun> parent_t::contGen{};
template <>
std::map<std::string, std::string> parent_t::contUsage{};

void CandidateMethodFactory::init()
{
// TODO: add new strategy here
	registerInOne<CandidateMthdFreq>();
//	registerInOne<CandidateMthdInfreq>();
}

CandidateMethod * CandidateMethodFactory::generate(const std::string & name)
{
	FactoryProductTemplate* p = parent_t::generate(name);
	CandidateMethod* res = dynamic_cast<CandidateMethod*>(p);
	return res;
}
