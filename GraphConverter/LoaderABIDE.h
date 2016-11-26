#pragma once
#include "TCLoader.h"
#include <tuple>

class QCChecker;
class LoaderABIDE
	:public TCLoader

{

	static const int POS_ID, POS_DX;
//	static const std::string filePrefix;
	static const std::vector<std::string> header;
	static std::vector<int> POS_QC; // qc_rater_1, qc_anat_rater_2, qc_func_rater_2, qc_anat_rater_3
	static bool FLG_QC_SET;
//	static const int ID_LENGTH_FILE = 7;
//	static const char PADDING = '0';
public:
	virtual std::vector<SubjectInfo> loadSubjectsFromDescFile(const std::string& fn,
		const std::string& qcMethod, const int nSubject = -1, const int nSkip = 0);

	virtual std::vector<SubjectInfo> pruneAndAddScanViaScanFile(
		std::vector<SubjectInfo>& vldList, const std::string& root);

	virtual std::string getFilePath(const SubjectInfo& sub);

	virtual tc_t loadTimeCourse(const std::string& fn);
private:
	static void InitPOS_QC();
	bool checkHeader(const std::string& line);
	std::tuple<bool, std::string, int> parsePhenotypeLine(
		const std::string& line, QCChecker* pchecker);
};


