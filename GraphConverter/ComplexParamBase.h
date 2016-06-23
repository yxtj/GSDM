#pragma once
#include "Option.h"
#include <vector>
#include <string>

class ComplexParamBase
{
protected:
	std::vector<std::string> param;
public:
	void reg(Option& opt, const std::string& name, const std::string& des);
	virtual bool parse() = 0;
};

