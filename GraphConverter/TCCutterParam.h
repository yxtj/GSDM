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
	int nEach = -1;
	int nTotal = -1;
	int nStep = -1;
public:
	void reg(Option& opt);
	virtual bool parse();
private:
	bool parseEach();
	bool parseTotal();
	bool parseSlide();
	void numberCheck(int required) noexcept(false);
};

