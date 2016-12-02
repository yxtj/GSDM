#include "stdafx.h"
#include "TesterFactory.h"
#include "TesterIndividual.h"
#include "TesterGroup.h"

using namespace std;

TesterBase * TesterFactory::generate(const std::vector<std::string> & params)
{
	const string& name = params[0];
	TesterBase* res = nullptr;
	if(name == "individual") {
		res = new TesterIndividual(params);
	} else if(name == "group") {
		res = new TesterGroup(params);
	}
	return res;
}

