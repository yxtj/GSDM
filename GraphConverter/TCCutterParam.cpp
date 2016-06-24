#include "stdafx.h"
#include "TCCutterParam.h"
#include "Option.h"

using namespace std;

void TCCutterParam::reg(Option & opt)
{
	ComplexParamBase::reg(opt, "cut-method", "cutting method, should be one of the following:\n"
		"  nEach <size of each>, fix # of points of each piece\n"
		"  nTotal <# of total>, fix the total # of pieces\n"
		"  slide <size of window> <size of step>, use slide-window method"
	);
//	function<bool()> f = bind(&CutterParam::parse, this);
//	opt.addParser(f);
}

bool TCCutterParam::parse()
{
	if(param.empty())
		return false;
	method = param[0];
	bool res = false;
	if(method == "each") {
		res = parseEach();
	} else if(method == "total") {
		res = parseTotal();
	} else if(method == "slide") {
		res = parseSlide();
	} else {
		throw invalid_argument("Do not support this method");
	}
	return res;
}

bool TCCutterParam::parseEach()
{
	numberCheck(1);
	nEach = stoi(param[1]);
	return nEach > 0;
}

bool TCCutterParam::parseTotal()
{
	numberCheck(1);
	nTotal = stoi(param[1]);
	return nTotal > 0;
}

bool TCCutterParam::parseSlide()
{
	numberCheck(2);
	nEach = stoi(param[1]);
	nStep = stoi(param[2]);
	return nEach > 0 && nStep > 0;
}

void TCCutterParam::numberCheck(int required)
{
	if(param.size() != required + 1) {
		throw invalid_argument("Wrong number of parameters for " + name + " "
			+ method + " : " + to_string(required) + " required, but " 
			+ to_string(param.size() - 1) + " is given.");
	}
}
