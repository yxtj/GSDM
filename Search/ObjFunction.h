#pragma once
#include <string>
#include <map>
class ObjFunction
{
	int totalPos, totalNeg;
	double alpha;
public:
	enum class OFType{
		NONE, DIFF, MARGIN, RATIO
	};
	// none, diff, margin, ratio
	static const std::map<std::string, OFType> names;
	static const std::string usage;
private:
	OFType OFID;

public:
	ObjFunction();
	void setFunc(OFType type);
	void setFunc(const std::string& name);
	void setTotalPos(const int num);
	void setTotalNeg(const int num);
	void setAlpha(const double val);

	double operator()(int cntPos, int cntNeg) const;
	double operator()(int cntPos, int totalPos, int cntNeg, int totalNeg) const;
	double operator()(double freqPos, double freqNeg) const;

	/* Detailed objective functions */
private:
	using objFun_t = double(ObjFunction::*)(double, double)const;
	objFun_t pf;
public:
	double objFun_diffP2N(const double freqPos, const double freqNeg) const;
	double objFun_marginP2N(const double freqPos, const double freqNeg) const;
	double objFun_ratioP2N(const double freqPos, const double freqNeg) const;
};

