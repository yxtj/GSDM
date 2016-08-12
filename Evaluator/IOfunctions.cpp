#include "stdafx.h"
#include "IOfunctions.h"

using namespace std;

//---------------------------- Time Course ---------------------------------


//---------------------------- Correlation ---------------------------------

std::string genCorrFilename(const Subject & sub)
{
	return to_string(sub.type) + "-" + sub.id + "-" + to_string(sub.sgId) + ".txt";
}

bool checkCorrFilename(const string & fn)
{
	return checknParseCorrFilename(fn, nullptr);
}

Subject parseCorrFilename(const std::string & fn)
{
	size_t p1 = fn.find('-');
	size_t p2 = fn.find('-', p1 + 1);
	size_t pend = fn.rfind(".txt", string::npos, 4);
	return Subject(fn.substr(p1 + 1, p2 - p1 - 1), 
		stoi(fn.substr(0, p1)), stoi(fn.substr(p2 + 1, pend -p2 - 1)));
}

bool checknParseCorrFilename(const std::string& fn, Subject* pRes) noexcept
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
			pRes->id = fn.substr(p1 + 1, p2 -p1 -1);
			pRes->type = type;
			pRes->sgId = scanNum;
		}
	} catch(...) {
		return false;
	}
	return true;
}

//---------------------------- Graph ---------------------------------

std::string genGraphFilename(const Subject & sub)
{
	return genCorrFilename(sub);
}

bool checknParseGraphFilename(const std::string & fn, Subject * pRes) noexcept
{
	return checknParseCorrFilename(fn, pRes);
}



