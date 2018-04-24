#pragma once
#include <string>
#include <map>
class ObjFunction
{
	int totalPos, totalNeg;
	double alpha;
public:
	enum class OFType{
		NONE, DIFF, MARGIN, RATIO, GTEST
	};
	// none, diff, margin, ratio, g-test
	static const std::map<std::string, OFType> names;
	static std::string getUsage();
private:
	OFType OFID;

public:
	ObjFunction();
	explicit ObjFunction(const std::string& func_str);
	void setFuncType(OFType type);
	void setFunc(const std::string& func_str);
	OFType getFunc() const;
	std::string getFuncName() const;

	void setTotalPos(const int num);
	void setTotalNeg(const int num);
	bool needAlpha() const;
	void setAlpha(const double val);
	double getAlpha() const;

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
	double objFun_gtestP2N(const double freqPos, const double freqNeg) const;
	double objFun_gtest(const int nPos, const int nNeg) const;
};

