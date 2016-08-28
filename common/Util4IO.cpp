#include "stdafx.h"
#include "Util4IO.h"

using namespace std;

bool Util4IO::checkFilename2Subject(const string & fn)
{
	return checknParseFilename2Subject(fn, nullptr);
}


SubjectInfo Util4IO::parseFilename2Subject(const std::string & fn)
{
	size_t p1 = fn.find('-');
	size_t p2 = fn.find('-', p1 + 1);
	size_t pend = fn.rfind(".txt", string::npos, 4);
	return SubjectInfo(fn.substr(p1 + 1, p2 - p1 - 1),
		stoi(fn.substr(0, p1)), stoi(fn.substr(p2 + 1, pend - p2 - 1)));
}

bool Util4IO::checknParseFilename2Subject(const std::string& fn, SubjectInfo* pRes) noexcept
{
	if(fn.empty())
		return false;
	size_t p1 = fn.find('-');
	size_t p2 = fn.find('-', p1 + 1);
	size_t pend = fn.rfind(".txt", string::npos, 4);
	if(p1 == string::npos || p2 == string::npos || pend == string::npos)
		return false;
	try {
		int type = stoi(fn.substr(0, p1));
		int scanNum = stoi(fn.substr(p2 + 1, pend - p2 - 1));
		// keep old res if this operation cannot finish successfully
		if(pRes) {
			pRes->id = fn.substr(p1 + 1, p2 - p1 - 1);
			pRes->type = type;
			pRes->sgId = scanNum;
		}
	} catch(...) {
		return false;
	}
	return true;
}

