#pragma once
#include "TCLoader.h"
#include <tuple>

class QCChecker;
// Type: 0=Control; 1=ADHD-Combined; 2=ADHD-Hyperactive/Impulsive; 3=ADHD-Inattentive
class LoaderADHD200
	: public TCLoader
{
	static const std::string filePrefix;
	static const std::vector<std::string> header1, header2;
	static const int POS_ID, POS_DX;
	static const std::vector<int> POS_QC_1, POS_QC_2;

	static const int ID_LENGTH_FILE = 7;
	static const char PADDING = '0';
public:
	// 1st column: scan id, 6th column: Diagnosis (0-control, 1-ADHD-combined,
	//  2-ADHD-Hyperactive/Impulsive, 3-ADHD-Inattentive)
	// 18th column: QC_Rest_1, 22nd column: QC_Anatomical_1
	// return list of subject object (scan id, type)
	virtual std::vector<SubjectInfo> loadSubjectsFromDescFile(const std::string& fn,
		const std::string& qcMethod, const int nSubject = -1, const int nSkip = 0);

	virtual std::vector<SubjectInfo> pruneAndAddScanViaScanFile(std::vector<SubjectInfo>& vldList, const std::string& root);

	virtual std::string getFilePath(const SubjectInfo& sub);

	// 1st column: file, 2nd column: sequence id, 3rd~end: data 
	virtual tc_t loadTimeCourse(const std::string& fn);

private:
	int checkHeader(const std::string& line);
	// the folder and file names have 0 in high digits. total # of digits is 7
	std::string fixSubjectID(std::string id) const;
	// return <QC passed, scan id, diagnosis result>
	std::tuple<bool, std::string, int> parsePhenotypeLine(
		const std::string& line, const std::vector<int>* pPOS_QC, QCChecker* pchecker);
};

