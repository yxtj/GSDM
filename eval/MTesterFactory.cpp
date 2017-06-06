#include "stdafx.h"
#include "MTesterFactory.h"
#include "MTesterSingle.h"
#include "MTesterGroup.h"

using namespace std;

MTesterBase * MTesterFactory::generate(const std::vector<std::string> & params)
{
	const string& name = params[0];
	MTesterBase* res = nullptr;
	if(name == "single") {
		res = new MTesterSingle(params);
	} else if(name == "group") {
		res = new MTesterGroup(params);
	}
	return res;
}

std::string MTesterFactory::getName()
{
	return std::string("TesterMethod");
}

std::string MTesterFactory::getUsage()
{
	return MTesterSingle::usage + "\n"
		+ MTesterGroup::usage;
}

