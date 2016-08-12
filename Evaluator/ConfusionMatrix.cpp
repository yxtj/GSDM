#include "stdafx.h"
#include "ConfusionMatrix.h"

using namespace std;

ConfusionMatrix::ConfusionMatrix()
	:ConfusionMatrix(0,0,0,0)
{
}

ConfusionMatrix::ConfusionMatrix(int tp,  int fn, int fp,int tn)
	:tp(tp), fn(fn), fp(fp), tn(tn)
{
}

int ConfusionMatrix::numRealPos() const
{
	return tp + fn;
}

int ConfusionMatrix::numRealNeg() const
{
	return fp + tn;
}

int ConfusionMatrix::numPredPos() const
{
	return tp + fp;
}

int ConfusionMatrix::numPredNeg() const
{
	return fn + tn;
}

int ConfusionMatrix::numTrue() const
{
	return tp + tn;
}

int ConfusionMatrix::numFalse() const
{
	return fn + fp;
}

int ConfusionMatrix::numTotal() const
{
	return tp + fn + fp + tn;
}

double ConfusionMatrix::tpr() const
{
	return static_cast<double>(tp) / numRealPos();
}

double ConfusionMatrix::fpr() const
{
	return static_cast<double>(fp) / numRealNeg();
}

double ConfusionMatrix::tnr() const
{
	return static_cast<double>(tn) / numRealNeg();
}

double ConfusionMatrix::fnr() const
{
	return static_cast<double>(fn) / numRealPos();
}

double ConfusionMatrix::ppv() const
{
	return static_cast<double>(tp) / numPredPos();
}

double ConfusionMatrix::npv() const
{
	return static_cast<double>(tn) / numPredNeg();
}

double ConfusionMatrix::accuracy() const
{
	return static_cast<double>(tp + tn) / numTotal();
}

double ConfusionMatrix::f1() const
{
	return static_cast<double>(2 * tp) / (numRealPos() + numPredPos());
}

double ConfusionMatrix::fbeta(double beta) const
{
	double p = beta*beta;
	return ((1 + p)*tp) / (p*numRealPos() + numPredPos());
}

double ConfusionMatrix::mmc() const
{
	int up = tp*tn - fp*fn;
	double down1 = numTrue()*numFalse();
	double down2 = numRealPos()*numRealNeg();
	return up / sqrt(down1*down2);
}
