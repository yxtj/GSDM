#pragma once
class ConfusionMatrix
{
public:
	int tp, fn; // real positive
	int fp, tn; // real negative
public:
	ConfusionMatrix();
	ConfusionMatrix(int tp, int fn, int fp, int tn);
	
	int numRealPos() const;
	int numRealNeg() const;
	int numPredPos() const;
	int numPredNeg() const;
	int numTrue() const;
	int numFalse() const;
	int numTotal() const;

	double tpr() const;
	double fpr() const;
	double tnr() const;
	double fnr() const;
	// positive predictive value 
	double ppv() const;
	// negative predictive value 
	double npv() const;

	double recall() const {
		return tpr();
	}
	double precision() const {
		return ppv();
	}
	double accuracy() const;
	double f1() const;
	double fbeta(double beta) const;
	// Matthews correlation coefficient
	double mmc() const;

};

