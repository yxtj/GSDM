#pragma once
class Scorer
{
public:
	static double diff(const double fp, const double fn, const double alpha);
	static double ratio(const double fp, const double fn);
};

