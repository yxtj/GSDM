#include "stdafx.h"
#include "FactoryProductTemplate.h"

using namespace std;

void FactoryProductTemplate::checkNumber(int required, size_t given)
{
	if(given != required + 1) {
		throw invalid_argument("Wrong number of sub-options. "
			+ to_string(required + 1) + " required, but "
			+ to_string(given) + " is given.");
	}
}

void FactoryProductTemplate::checkName(const std::vector<string>& param, const std::string & name) noexcept(false)
{
	if(param[0] != name) {
		throw invalid_argument("The first sub-option (" + param.front()
			+ ") do not match expected name (" + name + ")");
	}
}


void FactoryProductTemplate::checkParam(const std::vector<string>& param, int reqired, const std::string & name) noexcept(false)
{
	try {
		checkNumber(reqired, param.size());
		checkName(param, name);
	} catch(exception e) {
		cerr << "error while checking sub-options for: " << name << endl;
		throw e;
	}
}
