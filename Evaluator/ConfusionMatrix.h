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

	// TP-rate: tp/(real pos) = tp/(tp+fn)
	double tpr() const;
	// FP-rate: fp/(real neg) = fp/(fp+tn)
	double fpr() const;
	// TN-rate: tn/(real neg) = tn/(fp+tn)
	double tnr() const;
	// FN-rate: fn/(rall pos) = fn/(tp+fn)
	double fnr() const;
	// positive predictive value: tp/(pred. pos) = tp/(tp+fp)
	double ppv() const;
	// negative predictive value: tn/(pred. neg) = tn/(fn+tn)
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

