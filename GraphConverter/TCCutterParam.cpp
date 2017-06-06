#include "stdafx.h"
#include "TCCutterParam.h"
#include "Option.h"

using namespace std;

const std::string TCCutterParam::name("cut-method");
const std::string TCCutterParam::usage(
	"The time course cutting method, should be one of the following:\n"
		"  whole, use the whole period\n"
		"  each <size of each>, fix # of points of each piece\n"
		"  total <# of total>, fix the total # of pieces\n"
		"  slide <size of window> <size of step>, use slide-window method"
);

bool TCCutterParam::parse(const std::vector<std::string>& params)
{
	if(params.empty())
		return true;
	method = params[0];
	bool res = false;
	if(method == "whole") {
		res = parseWhole(params);
	} else if(method == "each") {
		res = parseEach(params);
	} else if(method == "total") {
		res = parseTotal(params);
	} else if(method == "slide") {
		res = parseSlide(params);
	} else {
		throw invalid_argument("unknown parameter for cut-method: " + params[0]);
	}
	return res;
}

bool TCCutterParam::parseWhole(const std::vector<std::string>& params)
{
	numberCheck(0, params.size());
	return true;
}

bool TCCutterParam::parseEach(const std::vector<std::string>& params)
{
	numberCheck(1, params.size());
	nEach = stoi(params[1]);
	return nEach > 0;
}

bool TCCutterParam::parseTotal(const std::vector<std::string>& params)
{
	numberCheck(1, params.size());
	nTotal = stoi(params[1]);
	return nTotal > 0;
}

bool TCCutterParam::parseSlide(const std::vector<std::string>& params)
{
	numberCheck(2, params.size());
	nEach = stoi(params[1]);
	nStep = stoi(params[2]);
	return nEach > 0 && nStep > 0;
}

void TCCutterParam::numberCheck(int required, int given)
{
	if(given != required + 1) {
		throw invalid_argument("Wrong number of parameters for " + name + " "
			+ method + " : " + to_string(required) + " required, but " 
			+ to_string(given - 1) + " is given.");
	}
}
