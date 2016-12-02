#include "stdafx.h"
#include "TesterBase.h"

using namespace std;

void TesterBase::set(const Motif & m)
{
	setMark();
}

void TesterBase::set(const std::vector<Motif>& ms)
{
	setMark();
}

bool TesterBase::hasSet()
{
	return _hasSet;
}

void TesterBase::setMark()
{
	_hasSet = true;
}

TesterBase::~TesterBase()
{
}

void TesterBase::checkName(const std::string & given, const std::string & required)
{
	if(given != required) {
		throw invalid_argument("The first sub-option (" + given
			+ ") does not match the option : " + required);
	}
}

void TesterBase::checkNumParam(const int given, const int required)
{
	if(given != required) {
		throw invalid_argument("Wrong number of sub-options. "
			+ to_string(required) + " required, but "
			+ to_string(given) + " is given.");
	}
}

void TesterBase::checkNumParam(const int given, const int requiredMin, const int requiredMax)
{
	if(given < requiredMin || given > requiredMax) {
		throw invalid_argument("Wrong number of sub-options. "
			+ to_string(requiredMin) + " to " + to_string(requiredMax) + " required, but "
			+ to_string(given) + " is given.");
	}
}
