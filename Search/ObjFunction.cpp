#include "stdafx.h"
#include "ObjFunction.h"

using namespace std;

const std::map<std::string, ObjFunction::OFType> ObjFunction::names{
	{ "none", OFType::NONE }, { "diff", OFType::DIFF },
	{ "margin", OFType::MARGIN}, { "ratio", OFType::RATIO }
};
const std::string ObjFunction::usage{
	"Support: diff:<alpha>, margin:<alpha>, ratio:<alpha> . Default alpha=1."
};

ObjFunction::ObjFunction()
	:totalPos(0), totalNeg(0), alpha(1.0), OFID(OFType::NONE), pf(nullptr)
{
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
	auto it = names.find(name);
	if(it == names.end()) {
		throw invalid_argument("Unsupported objective function: " + name);
	}
	setFunc(it->second);
}

void ObjFunction::setTotalPos(const int num)
{
	totalPos = num;
}

void ObjFunction::setTotalNeg(const int num)
{
	totalNeg = num;
}
void ObjFunction::setAlpha(const double val)
{
	alpha = val;
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
