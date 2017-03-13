#include "stdafx.h"
#include "MTesterBase.h"

using namespace std;

void MTesterBase::set(const Motif & m)
{
	throw runtime_error("This MTester does not support set(Motif) funciton.");
}

void MTesterBase::set(const std::vector<Motif>& ms)
{
	throw runtime_error("This MTester does not support set(vector<Motif>) funciton.");
}

bool MTesterBase::hasSet()
{
	return _hasSet;
}

void MTesterBase::setMark()
{
	_hasSet = true;
}

MTesterBase::~MTesterBase()
{
}

void MTesterBase::checkName(const std::string & given, const std::string & required)
{
	if(given != required) {
		throw invalid_argument("The first sub-option (" + given
			+ ") does not match the option : " + required);
	}
}

void MTesterBase::checkNumParam(const int given, const int required)
{
	if(given != required) {
		throw invalid_argument("Wrong number of sub-options. "
			+ to_string(required) + " required, but "
			+ to_string(given) + " is given.");
	}
}

void MTesterBase::checkNumParam(const int given, const int requiredMin, const int requiredMax)
{
	if(given < requiredMin || given > requiredMax) {
		throw invalid_argument("Wrong number of sub-options. "
			+ to_string(requiredMin) + " to " + to_string(requiredMax) + " required, but "
			+ to_string(given) + " is given.");
	}
}
