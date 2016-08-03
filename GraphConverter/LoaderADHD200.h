#pragma once
#include "TCLoader.h"
#include <tuple>

class LoaderADHD200
	: public TCLoader
{
	static std::string filePrefix;
	static const std::vector<std::string> header;
	static const int ID_LENGTH_FILE = 7;
	static const char PADDING = '0';
public:
	// 1st column: scan id, 6th column: Diagnosis (0-control, 1-ADHD-combined,
	//  2-ADHD-Hyperactive/Impulsive, 3-ADHD-Inattentive)
	// 18th column: QC_Rest_1, 22nd column: QC_Anatomical_1
	// return list of subject object (scan id, type)
	virtual std::vector<Subject> loadValidList(const std::string& fn, const int nSubject = -1);

	virtual std::vector<Subject> getAllSubjects(std::vector<Subject>& vldList, const std::string& root);

	virtual std::string getFilePath(const Subject& sub);

	// 1st column: file, 2nd column: sequence id, 3rd~end: data 
	virtual tc_t loadTimeCourse(const std::string& fn);

private:
	bool checkHeader(const std::string& line);
	// the folder and file names have 0 in high digits. total # of digits is 7
	std::string fixSubjectID(std::string id) const;
	// return <QC passed, scan id, diagnosis result>
	std::tuple<bool, std::string, int> parsePhenotypeLine(const std::string& line);
};

