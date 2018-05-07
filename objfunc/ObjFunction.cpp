#include "ObjFunction.h"
#include <cmath>
#include <regex>
#include <algorithm>
#include <vector>
#include <stdexcept>

using namespace std;

const std::map<std::string, ObjFunction::OFType> ObjFunction::names{
	{ "none", OFType::NONE }, { "diff", OFType::DIFF },
	//{ "margin", OFType::MARGIN },
	{ "ratio", OFType::RATIO },
	{ "gtest", OFType::GTEST }
};

std::string ObjFunction::getUsage()
{
	return "Support: diff:<alpha>, margin:<alpha>, ratio:<alpha>, gtest. Default alpha=1.";
}

void ObjFunction::setFuncType(OFType type)
{
	OFID = type;
	switch(type)
	{
	case OFType::DIFF:
		pf = &ObjFunction::objFun_diffP2N;
		pu = &ObjFunction::ubFun_diff;
		pu2 = &ObjFunction::ubFun_diff2;
		break;
	// case OFType::MARGIN:
	// 	pf = &ObjFunction::objFun_marginP2N;
	// 	pu = &ObjFunction::ubFun_margin;
	// 	pu2 = &ObjFunction::ubFun_margin2;
	// 	break;
	case OFType::RATIO:
		pf = &ObjFunction::objFun_ratioP2N;
		pu = &ObjFunction::ubFun_ratio;
		pu2 = &ObjFunction::ubFun_ratio2;
		break;
	case OFType::GTEST:
		pf = &ObjFunction::objFun_gtestP2N;
		pu = &ObjFunction::ubFun_gtest;
		pu2 = &ObjFunction::ubFun_gtest2;
		break;
	case OFType::KLD:
		pf = &ObjFunction::objFun_kldP2N;
		pu = &ObjFunction::ubFun_kld;
		pu2 = &ObjFunction::ubFun_kld2;
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
	inv_tp = 1.0/totalPos;
}

void ObjFunction::setTotalNeg(const int num)
{
	totalNeg = num;
	inv_tn = 1.0/totalNeg;
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

double ObjFunction::score(double freqPos, double freqNeg) const
{
	return (this->*pf)(freqPos, freqNeg);
}
double ObjFunction::upperbound(double freqPos) const
{
	return (this->*pu)(freqPos);
}
double ObjFunction::upperbound(double freqPos, double freqNeg) const
{
	return (this->*pu2)(freqPos, freqNeg);
}

double ObjFunction::score(int cntPos, int cntNeg) const
{
	return score(static_cast<double>(cntPos) / totalPos,
		static_cast<double>(cntNeg) / totalNeg);
}
double ObjFunction::score(int cntPos, int totalPos, int cntNeg, int totalNeg) const
{
	return score(static_cast<double>(cntPos) / totalPos,
		static_cast<double>(cntNeg) / totalNeg);
}

// scoring functions:

double ObjFunction::objFun_diffP2N(const double freqPos, const double freqNeg) const
{
	return freqPos - alpha*freqNeg;
}

double ObjFunction::objFun_ratioP2N(const double freqPos, const double freqNeg) const
{
	//return freqNeg != 0.0 ? freqPos / freqNeg : freqPos;
	return freqPos*freqPos / (freqPos + freqNeg);
}

double ObjFunction::objFun_gtestP2N(const double freqPos, const double freqNeg) const
{
	double fn = freqNeg;
	if(fn == 0)
		fn = inv_tn;
	else if(fn == 1)
		fn = 1 - inv_tn;
	return freqPos*(freqPos*log(freqPos/fn) + (1-freqPos)*log((1-freqPos)/(1-fn)));
}
double ObjFunction::objFun_gtest(const int nPos, const int nNeg) const
{
	return 2*totalPos*log((nPos*totalNeg)/(nNeg*totalPos))
		+ 2*(totalPos-nPos)*log( (totalNeg*(totalPos-nPos))/(totalPos*(totalNeg-nNeg)) );
}

double ObjFunction::objFun_kldP2N(const double freqPos, const double freqNeg) const
{
	double fn = freqNeg;
	if(fn == 0)
		fn = inv_tn;
	return freqPos*freqPos*log(freqPos/fn);
}

// upper-bound functions:

double ObjFunction::ubFun_diff(const double freqPos) const
{
	return freqPos;
}
double ObjFunction::ubFun_diff2(const double freqPos, const double freqNeg) const
{
	// df/dx > 0 and df/dy < 0
	return freqPos;
}

double ObjFunction::ubFun_ratio(const double freqPos) const
{
	return freqPos;
}
double ObjFunction::ubFun_ratio2(const double freqPos, const double freqNeg) const
{
	// df/dx > 0 and df/dy < 0
	return freqPos;
}

double ObjFunction::ubFun_gtest(const double freqPos) const
{
	if(freqPos <= 0.5){
		return freqPos*(1-freqPos)*log((1-freqPos)/inv_tn);
	}else{
		return freqPos*freqPos*log(freqPos/inv_tn);
	}
}
double ObjFunction::ubFun_gtest2(const double freqPos, const double freqNeg) const
{
	return ubFun_gtest(freqPos);
}

double ObjFunction::ubFun_kld(const double freqPos) const
{
	return freqPos*log(freqPos/inv_tn);
}
double ObjFunction::ubFun_kld2(const double freqPos, const double freqNeg) const
{
	// df/dx > 0 and df/dy < 0
	return objFun_kldP2N(freqPos, freqNeg);
}
