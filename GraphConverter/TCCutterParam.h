#pragma once
#include "ComplexParamBase.h"
#include <string>

class Option;

struct TCCutterParam
	:public ComplexParamBase
{
	std::string method;
	// M-each use: nEach
	// M-total use: nTotal
	// M-slide use: nEach, nStep
	int nEach;
	int nTotal;
	int nStep;
public:
	void reg(Option& opt);
	virtual bool parse();
private:
	bool parseEach();
	bool parseTotal();
	bool parseSlide();
	void numberCheck(int required) noexcept(false);
};

