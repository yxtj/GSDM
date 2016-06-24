#pragma once
#include <vector>
#include <string>

class Option;

class ComplexParamBase
{
protected:
	std::string name;
	std::vector<std::string> param;
public:
	void reg(Option& opt, const std::string& name, const std::string& des);
	virtual bool parse() = 0;
};

