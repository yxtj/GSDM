#pragma once
#include "LoaderBaseCPAC.h"

class QCChecker;
// Type: 0=Control; 1=AD; 2=SMC; 3=EMCI; 4=LMCI
class LoaderADNI :
	public LoaderBaseCPAC
{
	static const int POS_ID, POS_DX;
	static const std::vector<std::string> header;
	static const std::vector<int> POS_QC; // no qc field
	static const int ID_LENGTH_FILE;
	static const char PADDING;
public:
	virtual std::vector<SubjectInfo> loadSubjectsFromDescFile(const std::string& fn,
		const std::string& qcMethod, const int nSubject = -1, const int nSkip = 0);
	virtual std::vector<SubjectInfo> pruneAndAddScanViaScanFile(
		std::vector<SubjectInfo>& vldList, const std::string & root);
	virtual std::string getFilePath(const SubjectInfo & sub);

private:
	bool checkHeader(const std::string &line);
	std::tuple<bool, std::string, int> parsePhenotypeLine(
		const std::string& line, QCChecker* pchecker);
	// Control -> CN -> 0, Alzheimer's Disease -> AD -> 1, Significant Memory Concern -> SMC -> 2,
	// early Mild Cognitive Impairment -> EMCI -> 3, late Mild Cognitive Impairment -> LMCI -> 4
	int giveDXId(const std::string& dx);
};

