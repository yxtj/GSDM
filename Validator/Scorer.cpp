#include "Scorer.h"

double Scorer::diff(const double fp, const double fn, const double alpha)
{
	return fp - alpha*fn;
}

double Scorer::ratio(const double fp, const double fn)
{
	return fp*fp / (fp + fn);
}
