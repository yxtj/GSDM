#include "ObjFunction.h"
#include <cmath>
#include <regex>
#include <algorithm>
#include <vector>
#include <stdexcept>

using namespace std;

const std::map<std::string, ObjFunction::OFType> ObjFunction::names{
	{ "none", OFType::NONE }, { "diff", OFType::DIFF },
	{ "margin", OFType::MARGIN }, { "ratio", OFType::RATIO },
	{ "gtest", OFType::GTEST }
};

std::string ObjFunction::getUsage()
{
	return "Support: diff:<alpha>, margin:<alpha>, ratio:<alpha>, gtest. Default alpha=1.";
}

ObjFunction::ObjFunction()
	: totalPos(0), totalNeg(0), alpha(1.0), OFID(OFType::NONE), pf(nullptr)
{
}

ObjFunction::ObjFunction(const std::string & func_str)
	: totalPos(0), totalNeg(0), alpha(1.0), OFID(OFType::NONE), pf(nullptr)
{
	setFunc(func_str);
}

void ObjFunction::setFuncType(OFType type)
{
	OFID = type;
	switch(type)
	{
	case OFType::DIFF:
		pf = &ObjFunction::objFun_diffP2N;
		break;
	case OFType::MARGIN:
		pf = &ObjFunction::objFun_marginP2N;
		break;
	case OFType::RATIO:
		pf = &ObjFunction::objFun_ratioP2N;
		break;
	case OFType::GTEST:
		pf = &ObjFunction::objFun_gtestP2N;
		break;
	default:
		pf = nullptr;
		break;
	}
}

void ObjFunction::setFunc(const std::string & func_str)
{
	bool failed = true;
	smatch m;
	regex reg_obj("(\\w+)(:\\d?\\.?\\d*)?");
	if(regex_match(func_str, m, reg_obj)) {
		string temp(m[1].str());
		transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
		auto it = names.find(temp);
		if(it != names.end()){
			setFuncType(it->second);
			if(m[2].matched){
				try{
					setAlpha(stod(m[2].str().substr(1)));
				}catch(...){
					throw invalid_argument("Unable to parse objective function parameter in"+func_str);
				}
			}
		}
		failed = false;
	}

	if(failed){
		throw invalid_argument("Unsupported objective function: " + func_str);
	}
}

ObjFunction::OFType ObjFunction::getFunc() const
{
	return OFID;
}

std::string ObjFunction::getFuncName() const
{
	for(auto& p : names) {
		if(p.second == OFID)
			return p.first;
	}
	return std::string();
}

void ObjFunction::setTotalPos(const int num)
{
	totalPos = num;
}

void ObjFunction::setTotalNeg(const int num)
{
	totalNeg = num;
}
bool ObjFunction::needAlpha() const
{
	static vector<OFType> OF_NEED_PARAMTER =
		{ OFType::DIFF, OFType::MARGIN, OFType::RATIO };
	return find(OF_NEED_PARAMTER.begin(), OF_NEED_PARAMTER.end(), OFID) != OF_NEED_PARAMTER.end();
}
void ObjFunction::setAlpha(const double val)
{
	alpha = val;
}
double ObjFunction::getAlpha() const
{
	return alpha;
}
double ObjFunction::operator()(int cntPos, int cntNeg) const
{
	return this->operator()(
		static_cast<double>(cntPos) / totalPos,
		static_cast<double>(cntNeg) / totalNeg
		);
}
double ObjFunction::operator()(int cntPos, int totalPos, int cntNeg, int totalNeg) const
{
	return this->operator()(
		static_cast<double>(cntPos) / totalPos,
		static_cast<double>(cntNeg) / totalNeg
		);
}
double ObjFunction::operator()(double freqPos, double freqNeg) const
{
	return (this->*pf)(freqPos, freqNeg);
}
double ObjFunction::objFun_diffP2N(const double freqPos, const double freqNeg) const
{
	return freqPos - alpha*freqNeg;
}

double ObjFunction::objFun_marginP2N(const double freqPos, const double freqNeg) const
{
	return (1.0 - freqPos) + alpha*freqNeg;
}

double ObjFunction::objFun_ratioP2N(const double freqPos, const double freqNeg) const
{
	//return freqNeg != 0.0 ? freqPos / freqNeg : freqPos;
	return freqPos*freqPos / (freqPos + freqNeg);
}

double ObjFunction::objFun_gtestP2N(const double freqPos, const double freqNeg) const
{
	return 2*freqPos*log(freqPos/freqNeg) + 2*(1-freqPos)*log((1-freqPos)/(1-freqNeg));
}
double ObjFunction::objFun_gtest(const int nPos, const int nNeg) const
{
	return 2*totalPos*log((nPos*totalNeg)/(nNeg*totalPos))
		+ 2*(totalPos-nPos)*log( (totalNeg*(totalPos-nPos))/(totalPos*(totalNeg-nNeg)) );
}