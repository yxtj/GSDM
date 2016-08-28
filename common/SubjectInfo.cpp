#include "stdafx.h"
#include "SubjectInfo.h"

using namespace std;

SubjectInfo::SubjectInfo(const std::string & filename)
{
	if(!parseFromFilename(filename))
		throw invalid_argument("given filename: \"" + filename + "\" cannot be parsed.");
}

SubjectInfo::SubjectInfo(const std::string & id, const int type, const int seqNum)
	:id(id), type(type), seqNum(seqNum)
{
}

SubjectInfo::SubjectInfo(std::string && id, const int type, const int seqNum)
	:id(move(id)), type(type), seqNum(seqNum)
{
}

std::string SubjectInfo::genFilename() const
{
	return to_string(type) + "-" + id + "-" + to_string(seqNum) + ".txt";
}

// filename format: <type>-<subject Id>-<scan Id>.txt
static regex subFnPattern("^(\\d+)-([^-]+)-(\\d+)\\.txt$");

bool SubjectInfo::parseFromFilename(const std::string & fn)
{
	// regex-based method:
	smatch res;
	if(regex_match(fn, res, subFnPattern)) {
		type = stoi(res[1].str());
		id = res[2].str();
		seqNum = stoi(res[3].str());
		return true;
	}
	return false;
	// string-based method:
	if(fn.empty())
		return false;
	size_t p1 = fn.find('-');
	size_t p2 = fn.find('-', p1 + 1);
	size_t pend = fn.rfind(".txt", string::npos, 4);
	if(p1 == string::npos || p2 == string::npos || pend == string::npos)
		return false;
	try {
		id = fn.substr(p1 + 1, p2 - p1 - 1);
		type = stoi(fn.substr(0, p1));
		seqNum = stoi(fn.substr(p2 + 1, pend - p2 - 1));
	} catch(...) {
		return false;
	}
	return true;
}

bool SubjectInfo::checkFilename(const std::string & fn)
{
	return regex_match(fn, subFnPattern);
}
