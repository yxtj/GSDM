#include "ObjFunction.h"
#include <algorithm>

using namespace std;

const std::map<std::string, ObjFunction::OFType> ObjFunction::names{
	{ "none", OFType::NONE }, { "diff", OFType::DIFF },
	{ "margin", OFType::MARGIN}, { "ratio", OFType::RATIO }
};

std::string ObjFunction::getUsage()
{
	return "Support: diff:<alpha>, margin:<alpha>, ratio:<alpha> . Default alpha=1.";
}

ObjFunction::ObjFunction()
	: totalPos(0), totalNeg(0), alpha(1.0), OFID(OFType::NONE), pf(nullptr)
{
}

ObjFunction::ObjFunction(const std::string & name)
	: totalPos(0), totalNeg(0), alpha(1.0), OFID(OFType::NONE), pf(nullptr)
{
	setFunc(name);
}

void ObjFunction::setFunc(OFType type)
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
	default:
		pf = nullptr;
		break;
	}
}

void ObjFunction::setFunc(const std::string & name)
{
	string temp(name);
	transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
	auto it = names.find(temp);
	if(it == names.end()) {
		throw invalid_argument("Unsupported objective function: " + name);
	}
	setFunc(it->second);
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
	return OFID != OFType::NONE;
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
