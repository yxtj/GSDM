#include "stdafx.h"
#include "FactoryTemplate.h"

using namespace std;

std::string FactoryTemplate::optName("unset");
std::string FactoryTemplate::usagePrefix(
	"usage prefix is not set\n");

std::map<std::string, FactoryTemplate::createFun> FactoryTemplate::contGen{};
std::map<std::string, std::string> FactoryTemplate::contUsage{};

bool FactoryTemplate::isValid(const std::string & name)
{
	return contGen.find(name) != contGen.end();
}

void FactoryTemplate::init()
{
}

void FactoryTemplate::registerUsage(const std::string& name, const std::string& usage)
{
	contUsage[name] = usage;
}

std::string FactoryTemplate::getOptName()
{
	return optName;
}

void FactoryTemplate::setOptName(const std::string & name)
{
	optName = name;
}

std::string FactoryTemplate::getUsage()
{
	string res = usagePrefix;
	int cnt = 0;
	for(const auto& usg : contUsage) {
		res += "Option " + to_string(++cnt) + ": " + usg.first
			+ "\n" + usg.second + "\n";
	}
	return res;
}

void FactoryTemplate::setUsage(const std::string & usage)
{
	usagePrefix = usage;
}

FactoryProductTemplate * FactoryTemplate::generate(const std::string & name)
{
	FactoryProductTemplate* res = nullptr;
	if(isValid(name)) {
		res = contGen.at(name)();
	}
	return res;
}

