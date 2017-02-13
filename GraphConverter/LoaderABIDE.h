#pragma once
#include "TCLoader.h"
#include <tuple>
#include <map>

class QCChecker;
// Type: 1=Autism; 2=Control
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
private:
	std::map<std::string, std::string> nameMapping;
public:
	// the subject.id includes both SITE and ID
	virtual std::vector<SubjectInfo> loadSubjectsFromDescFile(const std::string& fn,
		const std::string& qcMethod, const int nSubject = -1, const int nSkip = 0);

	// the subject.id ONLY includes ID
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


