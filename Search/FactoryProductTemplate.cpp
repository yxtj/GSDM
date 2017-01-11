#include "stdafx.h"
#include "FactoryProductTemplate.h"

using namespace std;

void FactoryProductTemplate::checkNumber(size_t required, size_t given)
{
	given -= 1;
	if(given != required) {
		throw invalid_argument("Wrong number of sub-options. "
			+ to_string(required) + " required, but "
			+ to_string(given) + " is given.");
	}
}

void FactoryProductTemplate::checkNumber(size_t requiredMin, size_t requiredMax, size_t given)
{
	given -= 1;
	if(given < requiredMin || given > requiredMax) {
		throw invalid_argument("Wrong number of sub-options. "
			+ to_string(requiredMin) + " to " + to_string(requiredMax) + " required, but "
			+ to_string(given) + " is given.");
	}
}

void FactoryProductTemplate::checkName(const std::vector<string>& param, const std::string & name) noexcept(false)
{
	if(param.empty()) {
		throw invalid_argument("No parameter is given for the option : " + name);
	} else if (param[0] != name) {
		throw invalid_argument("The first sub-option (" + param.front()
			+ ") does not match the option : " + name);
	}
}


void FactoryProductTemplate::checkParam(const std::vector<string>& param,
	size_t reqired, const std::string & name)// noexcept(false)
{
	try {
		checkNumber(reqired, param.size());
		checkName(param, name);
	} catch(exception e) {
		cerr << "error while checking sub-options for: " << name << endl;
		throw e;
	}
}

void FactoryProductTemplate::checkParam(const std::vector<std::string>& param,
	size_t reqiredMin, size_t requiredMax, const std::string & name)// noexcept(false)
{
	try {
		checkNumber(reqiredMin, requiredMax, param.size());
		checkName(param, name);
	} catch(exception e) {
		cerr << "error while checking sub-options for: " << name << endl;
		throw e;
	}
}
