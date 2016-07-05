#include "stdafx.h"
#include "CandidateMethodFactory.h"

// include conclude strategies:
#include "CandidateMthdFreq.h"
//#include "CandidateMthdInfreq.h"

using namespace std;

const std::string CandidateMethodFactory::optName("method");
const std::string CandidateMethodFactory::usagePrefix(
	"Use the following parameters to select ONE searching method\n");

std::map<std::string, CandidateMethodFactory::createFun> CandidateMethodFactory::contGen{};
std::map<std::string, std::string> CandidateMethodFactory::contUsage{};

bool CandidateMethodFactory::isValid(const std::string & name)
{
	return contGen.find(name) != contGen.end();
}

void CandidateMethodFactory::init()
{
	registerClass<CandidateMthdFreq>();
	registerUsage<CandidateMthdFreq>();
	// TODO: add new strategy here
//	registerClass<CandidateMthdInfreq>();
}

void CandidateMethodFactory::registerUsage(const std::string& name, const std::string& usage)
{
	contUsage[name] = usage;
}

std::string CandidateMethodFactory::getOptName()
{
	return optName;
}

std::string CandidateMethodFactory::getUsage()
{
	string res = usagePrefix;
	int cnt = 0;
	for(const auto& usg : contUsage) {
		res += "Method " + to_string(++cnt) + ": " + usg.first
			+ "\n" + usg.second + "\n";
	}
	return res;
}


CandidateMethod * CandidateMethodFactory::generate(const std::string & methodName)
{
	CandidateMethod* res = nullptr;
	if(isValid(methodName)) {
		res = contGen.at(methodName)();
	}
	return res;
}

CandidateMethod * CandidateMethodFactory::generate(const CandidateMethodParam & methodParam)
{
	return generate(methodParam.name);
}
